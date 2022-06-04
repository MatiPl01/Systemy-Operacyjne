#include <errno.h>
#include "lib/libcommon.h"


#define MAX_CLIENTS_COUNT 16
const int PING_INTERVAL = 1000; // ms

client *clients[MAX_CLIENTS_COUNT];
int clients_count = 0;

int local_socket_fd;
struct sockaddr_un local_socket_address;

int network_socket_fd;
struct sockaddr_in network_socket_address;

int epoll_fd;

pthread_t requests_thread;
pthread_t connections_thread;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void get_input_args(int argc, char* argv[], int* port_number, char** socket_path);
int start_local_server(char* socket_path);
int start_network_server(int port_number);
int start_socket(int *socket_fd, int domain, struct sockaddr* socket_address, socklen_t socket_address_size);
int start_requests_handler(void);
int start_connections_handler(void);
pthread_t create_thread(void* routine, void* args);
_Noreturn void* connections_handler(void);
void remove_unavailable_clients(void);
void ping_clients(void);
void* requests_handler(void);
int close_connection(int socket_fd);
void close_servers(void);
int message_client(client *client, message msg);
int wait_until_shutdown(void);
int create_epoll(void);


int main(int argc, char* argv[]) {
    // Get server input arguments
    int port_number;
    char* socket_path = NULL;
    get_input_args(argc, argv, &port_number, &socket_path);

    // Set up the exit handler
    if (atexit(close_servers) == -1) {
        cperror("Cannot set the exit handler\n");
        free(socket_path);
        return EXIT_FAILURE;
    }

    // Start servers
    if (start_local_server(socket_path) == -1 || start_network_server(port_number) == -1
    // Start requests and client connections handlers
    || (start_requests_handler() == -1 || start_connections_handler() == -1
    // Wait until threads are closed
    || wait_until_shutdown() == -1)
    ) {
        free(socket_path);
        return EXIT_FAILURE;
    }

    // Release memory
    free(socket_path);

    return EXIT_SUCCESS;
}


void get_input_args(int argc, char* argv[], int* port_number, char** socket_path) {
    int i = 1;  // Start from the first argument (skip the program path)
    *port_number = get_input_num(&i, argc, argv, "Please provide TCP port number");
    *socket_path = get_input_string(&i, argc, argv, "Please provide UNIX socket path");
}

int start_local_server(char* socket_path) {
    // Bind the address to the local server socket
    local_socket_address.sun_family = AF_UNIX;

    printf("SIZE 1: %lu\n", sizeof(local_socket_address));

    if (start_socket(&local_socket_fd, AF_UNIX, (struct sockaddr*) &local_socket_address, sizeof(local_socket_address)) == -1) {
        cperror("Failed to start the local server\n");
        return -1;
    }

    cprintf(CYAN, "Local server is listening on path: %s\n", socket_path);
    return 0;
}

int start_network_server(int port_number) {
    // Bind the address to the network server socket
    network_socket_address.sin_family = AF_INET;
    network_socket_address.sin_port = htons(port_number);
    network_socket_address.sin_addr.s_addr = inet_addr(LOCALHOST);

    // Start the network server
    if (start_socket(&network_socket_fd, AF_INET, (struct sockaddr*) &network_socket_address, sizeof(network_socket_address)) == -1) {
        cperror("Failed to start the network server\n");
        return -1;
    }

    cprintf(CYAN, "Network server is listening on port: %d\n", port_number);
    return 0;
}

int start_socket(int *socket_fd, int domain, struct sockaddr* socket_address, socklen_t socket_address_size) {
    // Create a socket
    if ((*socket_fd = socket(domain, SOCK_STREAM, 0)) == -1) {
        cperror("Cannot create a socket\n");
        return -1;
    }

    // Set socket options
    int optval = 1;
    setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval));

    // Bind the address to the socket
    if (bind(*socket_fd, socket_address, socket_address_size) == -1) {
        cperror("Cannot bind a socket\n");
        return -1;
    }

    if (listen(*socket_fd, MAX_CLIENTS_COUNT) == -1) {
        cperror("Cannot start listening on a socket\n");
        return -1;
    }

    return 0;
}

int close_connection(int socket_fd) {
    if (shutdown(socket_fd, SHUT_RDWR) == -1) {
        cperror("Cannot shutdown the socket connection\n");
        return -1;
    }
    if (close(socket_fd) == -1) {
        cperror("Cannot close the socket descriptor\n");
        return -1;
    }

    return 0;
}

void close_servers(void) {
    cprintf(CYAN, "Shutting down servers...\n");
    if (close_connection(local_socket_fd) == -1 || close_connection(network_socket_fd) == -1) {
        exit(EXIT_FAILURE);
    }
}

pthread_t create_thread(void* routine, void* args) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, routine, args) != 0) {
        cperror("Cannot create a thread\n");
        return -1;
    }
    return thread;
}

int start_requests_handler(void) {
    if ((requests_thread = create_thread(requests_handler, NULL)) == -1) {
        cperror("Cannot set up the requests handler\n");
        return -1;
    }
    return 0;
}

int start_connections_handler(void) {
    if ((connections_thread = create_thread(connections_handler, NULL)) == -1) {
        cperror("Cannot set up the connections handler\n");
        return -1;
    }
    return 0;
}

int message_client(client *client, message msg) {
    if (send_message(client->socket_fd, msg) == -1) {
        cfprintf(stderr, RED, "Cannot send a message to %d\n", client->nickname);
        return -1;
    }
    return 0;
}

_Noreturn void* connections_handler(void) {
    cprintf(CYAN, "Waiting for client connections...\n");

    while (true) {
        usleep(1000 * PING_INTERVAL);
        if (clients_count == 0) continue;

        pthread_mutex_lock(&clients_mutex);
        remove_unavailable_clients();
        ping_clients();
        pthread_mutex_unlock(&clients_mutex);
    }
}

void remove_unavailable_clients(void) {
    for (int i = 0; i < MAX_CLIENTS_COUNT; i++) {
        if (!clients[i]) continue;
        if (!clients[i]->is_available) {
            cprintf(YELLOW, "Client %s is not responding. Removing this client...\n", clients[i]->nickname);
            if (close_connection(clients[i]->socket_fd) == -1) {
                cfprintf(stderr, RED, "%s client connection cannot be shut down\n", clients[i]->nickname);
            }
            clients[i] = NULL;
        }
    }
}

void ping_clients(void) {
    cprintf(CYAN, "Pinging clients...\n");

    for (int i = 0; i < MAX_CLIENTS_COUNT; i++) {
        if (!clients[i]) continue;
        message msg = { .type = PING };
        message_client(clients[i], msg);
        clients[i]->is_available = false;
    }
}

void* requests_handler(void) {

}

int wait_until_shutdown(void) {
    if (pthread_join(connections_thread, NULL) != 0) {
        cperror("Cannot join the connection thread\n");
        return -1;
    }

    if (pthread_join(requests_thread, NULL) != 0) {
        cperror("Cannot join the requests thread\n");
        return -1;
    }

    return 0;
}

int create_epoll(void) {
    if ((epoll_fd = epoll_create1(0)) == -1) {
        cperror("Cannot create the epoll\n");
        return -1;
    }

    return 0;
}
