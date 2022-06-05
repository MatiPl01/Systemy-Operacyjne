#include "lib/libcommon.h"
#include "client.h"


int epoll_fd = -1;
int socket_fd = -1;

int get_input_args(int argc, char* argv[], char* *nickname, char* *connection_method, char* *server_address);
int validate_connection_method(char* connection_method);
int connect_server(char* connection_method, char* server_address);
int connect_local_server(char* server_address);
int connect_network_server(char* server_address);
int connect_to_socket(int domain, struct sockaddr* socket_address, socklen_t address_size);
int disconnect_server(void);
int create_epoll(void);
void exit_handler(void);
void sigint_handler(int sig_no);


int main(int argc, char* argv[]) {
    // Get client input arguments
    char* nickname = NULL;
    char* connection_method = NULL;
    char* server_address = NULL;

    if (get_input_args(argc, argv, &nickname, &connection_method, &server_address) == -1) {
        free_args(3, nickname, connection_method, server_address);
        return EXIT_FAILURE;
    }

    // Set up the exit handler
    if (atexit(exit_handler) == -1) {
        cperror("Cannot set the exit handler\n");
        free_args(3, nickname, connection_method, server_address);
        return EXIT_FAILURE;
    }

    // Set up the SIGINT handler
    if (set_sa_handler(SIGINT, 0, sigint_handler) == -1) {  // TODO - check if this works
        cperror("Cannot set the SIGINT handler\n");
        free_args(3, nickname, connection_method, server_address);
        return EXIT_FAILURE;
    }

    // Connect to the server
    if (connect_server(connection_method, server_address) == -1) {
        free_args(3, nickname, connection_method, server_address);
        return EXIT_FAILURE;
    }

    // Release memory
    free_args(3, nickname, connection_method, server_address);

    return EXIT_SUCCESS;
}


int get_input_args(int argc, char* argv[], char* *nickname, char* *connection_method, char* *server_address) {
    int i = 1;  // Start from the first argument (skip the program path)
    *nickname = get_input_string(&i, argc, argv,
        "Please provide your nickname (displayed name)"
    );
    *connection_method = get_input_string(&i, argc, argv,
        "Please choose your connection method.\n"
        "Available methods are listed below:\n"
        "  - network\n"
        "  - local"
    );

    if (validate_connection_method(*connection_method) == -1) return -1;

    *server_address = get_input_string(&i, argc, argv,
        strcmp(*connection_method, NETWORK) == 0 ?
        "Please provide server address (IPv4 and port number) [IPv4:PORT]" :
        "Please provide server address (path to the UNIX server)"
    );

    return 0;
}

int validate_connection_method(char* connection_method) {
    if (strcmp(connection_method, NETWORK) != 0 && strcmp(connection_method, LOCAL) != 0) {
        cerror("Wrong connection method specified. Shutting down...");
        return -1;
    }
    return 0;
}

int connect_server(char* connection_method, char* server_address) {
    if (strcmp(connection_method, LOCAL) == 0) {
        return connect_local_server(server_address);
    } else {
        return connect_network_server(server_address);
    }
}

int connect_local_server(char* server_address) {
    struct sockaddr_un address;
//    memset(&address, 0, sizeof(address));  // TODO - check if calloc will work / maybe add some error checking
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, server_address);
    return connect_to_socket(AF_UNIX, (struct sockaddr*) &address, sizeof address);
}

int connect_network_server(char* server_address) {
    struct sockaddr_in address;
//    memset(&address, 0, sizeof(address));  // TODO - check if calloc will work / maybe add some error checking
    char* port;
    char* ipv4 = strtok_r(server_address, ":", &port);

    address.sin_family = AF_INET;
    address.sin_port = htons((int) strtol(port, NULL, 10));
    if (inet_pton(AF_INET, ipv4, &address.sin_addr) != 1) {
        cerror("Invalid network server address was specified\n");
        return -1;
    }
    return connect_to_socket(AF_INET, (struct sockaddr*) &address, sizeof address);
}

int connect_to_socket(int domain, struct sockaddr* socket_address, socklen_t address_size) {
    // Open the specified socket
    if ((socket_fd = socket(domain, SOCK_STREAM, 0)) == -1) {
        cperror("Cannot open the specified socket\n");
        return -1;
    }
    // Connect to the socket that was opened
    if (connect(socket_fd, (struct sockaddr*) &socket_address, address_size) == -1) {
        cperror("Cannot connect to the specified socket\n");
        socket_fd = -1;  // Set the socket file descriptor to -1 to indicate error
        return -1;
    }
    return 0;
}

void exit_handler(void) {
    if (socket_fd != -1) disconnect_server();
}

void sigint_handler(int sig_no) {
    cinfo("Received %d signal\n", sig_no);
    exit(EXIT_SUCCESS);
}

int disconnect_server(void) {
    cinfo("Disconnecting the server...\n");
    message msg = { .type = DISCONNECT };
    write(socket_fd, &msg, sizeof msg);
    exit(EXIT_SUCCESS);
}

int create_epoll(void) {
    if ((epoll_fd = epoll_create1(0)) == -1) {
        cperror("Cannot create the epoll\n");
        return -1;
    }
    // Set up epoll connection-related events
    // (The server will write connection-related events to the client socket_fd)
    struct epoll_event connection_event = {
        .events = EPOLLIN | EPOLLPRI | EPOLLHUP,
        .data = { .fd = socket_fd }
    };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &connection_event) == -1) {
        cperror("Cannot add the connection event to the epoll\n");
        epoll_fd = -1;
        return -1;
    }
    // Set up epoll game-related events
    // (The server will write game-related events to the client standard input)
    struct epoll_event game_event = {
        .events = EPOLLIN | EPOLLPRI,
        .data = { .fd = STDIN_FILENO }
    };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &game_event) == -1) {
        cperror("Cannot add the game event to the epoll\n");
        epoll_fd = -1;
        return -1;
    }

    return 0;
}
