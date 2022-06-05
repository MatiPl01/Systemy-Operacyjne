#include "lib/libcommon.h"
#include "server.h"


client *waiting_client;  // A client waiting for an opponent
connection connections[MAX_CONNECTIONS];
int connections_count = 0;

int epoll_fd;
int local_socket_fd;
int network_socket_fd;

pthread_t ping_thread;
pthread_t requests_thread;
pthread_mutex_t connections_mutex = PTHREAD_MUTEX_INITIALIZER;

// Server input arguments
int port_number;
char* socket_path;


void get_input_args(int argc, char* argv[]);
void exit_handler(void);
void sigint_handler(int sig_no);
void init_servers(void);
void init_local_server(void);
void init_network_server(void);
void init_server_socket(int socket_fd, struct sockaddr* addr, socklen_t addr_size);
void start_server_threads(void);
_Noreturn void* ping_handler(void);
int ping_connections(void);
int ping_connection(connection connection);
int remove_connection(connection *conn);
_Noreturn void* requests_handler(void);
void handle_epoll_event(struct epoll_event *epoll_event);
void handle_new_connection_request(event* request_event);
connection* create_connection(int connection_socket_fd);
connection* find_empty_connection(void);
void handle_client_request(event* request_event);

void print_startup_info(void);

void join_server_threads(void);


int main(int argc, char* argv[]) {
    // Get server input arguments
    get_input_args(argc, argv);
    // Set up the exit handler
    catch_perror(atexit(exit_handler));
    // Set up the SIGINT handler
    catch_perror(set_sa_handler(SIGINT, 0, sigint_handler));
    // Initialize servers
    init_servers();
    // Start server threads
    start_server_threads();
    // Print startup info
    print_startup_info();


    // Join the server threads
    join_server_threads();  // TODO - check this this works

    return EXIT_SUCCESS;
}


void get_input_args(int argc, char* argv[]) {
    int i = 1;  // Start from the first argument (skip the program path)
    port_number = get_input_num(&i, argc, argv, "Please provide TCP port number");
    socket_path = get_input_string(&i, argc, argv, "Please provide UNIX socket path");
}

void exit_handler(void) {
    cinfo("Shutting down...\n");
    free_args(1, socket_path);
}

void sigint_handler(int sig_no) {
    cinfo("Received %d signal\n", sig_no);
    exit(EXIT_SUCCESS);
}

void init_servers(void) {
    // Print info
    cinfo("Starting servers...\n");
    // Create the epoll
    epoll_fd = catch_perror(epoll_create1(0));
    // Initialize and start the local server
    init_local_server();
    // Initialize and start the network server
    init_network_server();
}

void init_local_server(void) {
    // Create the local server socket address structure
    struct sockaddr_un local_server_addr = { .sun_family = AF_UNIX };
    strcpy(local_server_addr.sun_path, socket_path);
    // Open and initialize the local server socket
    local_socket_fd = catch_perror(socket(AF_UNIX, SOCK_STREAM, 0));
    init_server_socket(local_socket_fd, (struct sockaddr*) &local_server_addr, sizeof(local_server_addr));
    // Unlink the local server socket path
    catch_perror(unlink(socket_path));
}

void init_network_server(void) {
    // Create the network server socket address structure
    struct sockaddr_in network_server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port_number),
        .sin_addr.s_addr = htonl(INADDR_ANY)  // Allow connections from any local interface
    };
    // Open and initialize the network server socket
    network_socket_fd = catch_perror(socket(AF_INET, SOCK_STREAM, 0));
    init_server_socket(network_socket_fd, (struct sockaddr*) &network_server_addr, sizeof(network_server_addr));
}

void init_server_socket(int socket_fd, struct sockaddr* addr, socklen_t addr_size) {
    // Bind the server socket address structure to the socket
    catch_perror(bind(socket_fd, addr, addr_size));
    // Start listening requests on the server socket
    catch_perror(listen(socket_fd, MAX_CONNECTIONS));
    // Set up the new connection event (enable new connection requests)
    struct epoll_event epoll_event = { .events = EPOLLIN | EPOLLPRI };  // TODO - check if EPOLLPRI is necessary
    event *event = epoll_event.data.ptr = malloc(sizeof *event);  // TODO - check if malloc is necessary
    event->type = NEW_CONNECTION;
    event->data.server_socket_fd = socket_fd;
    catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &epoll_event));
}

void start_server_threads(void) {
    // Create the ping handler thread
    if (pthread_create(&ping_thread, NULL, (void *(*)(void *)) ping_handler, NULL) != 0) {
        exit_error("Cannot create the ping handler thread\n");
    }
    // Create the request handler thread
    if (pthread_create(&requests_thread, NULL, (void *(*)(void *)) requests_handler, NULL) != 0) {
        exit_error("Cannot create the requests handler thread\n");
    }
}

_Noreturn void* ping_handler(void) {
    // Run the pinging loop
    while (true) {
        usleep(1000 * PING_INTERVAL);
        pthread_mutex_lock(&connections_mutex);
        int ret_val = ping_connections();
        pthread_mutex_unlock(&connections_mutex);
        if (ret_val == -1) exit_error("Something went wrong while pinging clients\n");
    }
}

int ping_connections(void) {
    cinfo("Pinging clients...\n");

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        // Continue if there is no connection on the current slot
        if (connections[i].status == EMPTY) continue;
        // Remove the connection if there was no response from a client
        // (status is still set to PINGED)
        if (connections[i].status == PINGED) {
            cwarn("%s%s%s is not responding\n", COLORS[MAGENTA], connections[i].client->nickname, COLORS[RESET]);
            if (remove_connection(&connections[i]) == -1) return -1;
        }
        // Otherwise, ping the current connection
        else if (ping_connection(connections[i]) == -1) return -1;
    }
    return 0;
}

int ping_connection(connection connection) {
    message msg = { .type = PING };
    connection.status = PINGED;
    if (write(connection.client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot to ping %s%s%s\n", COLORS[MAGENTA], connection.client->nickname, COLORS[RESET]);
        return -1;
    }
    return 0;
}

int remove_connection(connection *conn) {
    // Print info
    client *client = conn->client;
    cinfo("Removing %s%s%s connection...\n", COLORS[MAGENTA], client->nickname, COLORS[RESET]);

    // Remove connection data
    connection *opponent_connection = client->opponent_connection;
    if (client == waiting_client) waiting_client = NULL;
    else if (opponent_connection) {
        client->opponent_connection = NULL;
        remove_connection(opponent_connection);  // TODO - maybe send some message that opponent is not responding and therefore this client is also kicked
        free(opponent_connection);
    }
    int connection_socket_fd = conn->client_socket_fd;
    conn->client = NULL;
    conn->status = EMPTY;
    conn->client_socket_fd = -1;

    // Close the client connection socket
    if (close(connection_socket_fd) == -1) {
        cperror("Cannot close %s%s%s the connection\n", COLORS[MAGENTA], client->nickname, COLORS[RESET]);
        free(client);
        return -1;
    }

    // Decrement the number of current connections
    connections_count--;
    free(client);
    return 0;
}

_Noreturn void* requests_handler(void) {
    // Create an array to store epoll event structs
    struct epoll_event events[MAX_EVENTS_COUNT];
    // Run the requests handling loop
    while (true) {
        int event_count = catch_perror(epoll_wait(epoll_fd, events, MAX_EVENTS_COUNT, -1));
        for (int i = 0; i < event_count; i++) handle_epoll_event(&events[i]);
    }
}

void handle_epoll_event(struct epoll_event *epoll_event) {
    event *request_event = epoll_event->data.ptr;
    // Create the new connection (before the client registers)
    if (request_event->type == NEW_CONNECTION) handle_new_connection_request(request_event);
    // Otherwise, handle the client request event
    else if (request_event->type == CLIENT_EVENT) {
        // Remove the connection if it is hanging up
        if (epoll_event->events & EPOLLHUP) {
            pthread_mutex_lock(&connections_mutex);
            remove_connection(request_event->data.connection);
            pthread_mutex_unlock(&connections_mutex);
        // Otherwise, handle the client request
        } else handle_client_request(request_event);
    }
}

void handle_new_connection_request(event* request_event) {
    int connection_socket_fd = catch_perror(accept(request_event->data.server_socket_fd, NULL, NULL));
    connection *conn;
    // Inform a client that the new connection could not be established if the server is full
    if (!(conn = create_connection(connection_socket_fd))) {
        cwarn("Could not establish the new connection. Server is overloaded\n");
        message msg = { .type = SERVER_FULL };
        // Send a message to the client
        catch_perror(write(connection_socket_fd, &msg, sizeof msg));
        // Close the client connection
        catch_perror(close(connection_socket_fd));
    // Otherwise, if the server is not full, start listening to the client requests
    } else {
        // Create the event struct
        event *client_event = malloc(sizeof *client_event);  // TODO - check if malloc is necessary
        client_event->type = CLIENT_EVENT;
        client_event->data.connection = conn;  // Associate the new client connection struct with the new event
        struct epoll_event epoll_event = { .events = EPOLLIN | EPOLLET | EPOLLHUP, .data = client_event };  // TODO - check if all these flags are necessary
        // Add this event to the listened events
        catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_socket_fd, &epoll_event));
    }
}

connection* create_connection(int connection_socket_fd) {
    connection *conn = NULL;
    pthread_mutex_lock(&connections_mutex);
    conn = find_empty_connection();
    if (conn) {
        conn->client_socket_fd = connection_socket_fd;
        conn->status = CONNECTED;
        // Increment the number of current connections
        connections_count++;
    }
    pthread_mutex_unlock(&connections_mutex);
    return conn;
}

connection* find_empty_connection(void) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].status == EMPTY) return &connections[i];
    }
    return NULL;
}

void handle_client_request(event* request_event) {

}

void print_startup_info(void) {
    // Print the local server startup info
    cinfo("Local server is listening on %s%s%s\n", COLORS[MAGENTA], socket_path, COLORS[RESET]);
    // Print the network server startup info
    cinfo("Network server is listening on %s*:%d%s\n", COLORS[MAGENTA], port_number, COLORS[RESET]);
}




void join_server_threads(void) {  // TODO - check if this works
    // Join the connection thread
    if (pthread_join(ping_thread, NULL) != 0) {
        exit_error("Cannot join the connection thread\n");
    }
    // Join the requests thread
    if (pthread_join(requests_thread, NULL) != 0) {
        exit_error("Cannot join the requests thread\n");
    }
}
