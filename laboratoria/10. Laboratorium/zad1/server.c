#include "lib/libcommon.h"
#include "server.h"


connection *waiting_client_connection;  // A connection of a client waiting for an opponent
connection connections[MAX_CONNECTIONS];
int connections_count = 0;

int epoll_fd;  // Server's epoll file descriptor
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
void init_server_socket(int socket_fd, struct sockaddr *addr, socklen_t addr_size);
void start_server_threads(void);
_Noreturn void* ping_handler(void);
int ping_connections(void);
int ping_connection(connection connection);
int remove_connection(connection *conn);
_Noreturn void* requests_handler(void);
void handle_epoll_event(struct epoll_event *epoll_event);
void handle_new_connection_request(event *request_event);
connection* create_connection(int client_socket_fd);
connection* find_empty_connection(void);
void handle_client_request(event *request_event);
void register_client(connection *conn);
void create_client_struct(connection *conn, char* nickname);
bool is_nickname_available(char* nickname);
int start_game_between_clients(connection *conn1, connection *conn2);
void assign_random_symbols(char *symbol1, char *symbol2);
int send_game_start_msg(connection *conn, char symbol);
int wait_for_opponent(connection *conn);
void handle_client_ping_msg(connection *conn);
void handle_client_disconnect_msg(connection *conn);
void handle_client_move_msg(connection *conn, msg_data data);
bool is_valid_move(game_state *game_state, int move_id, char client_symbol);
void print_game_update_msg(connection *conn, char* format, ...);
int send_game_state(connection *conn);
game_result get_game_result(game_state *game_state, int move_id);
void print_game_result_msg(connection *conn, game_result game_result);
int send_game_result(connection *conn, game_result game_result);
game_result get_opposite_game_result(game_result game_result);
void print_startup_info(void);
void join_server_threads(void);


int main(int argc, char* argv[]) {
    srand(time(NULL));
    // Set up the exit handler
    catch_perror(atexit(exit_handler));
    // Set up the SIGINT handler
    catch_perror(set_sa_handler(SIGINT, 0, sigint_handler));
    // Get server input arguments
    get_input_args(argc, argv);
    // Initialize servers
    init_servers();
    // Start server threads
    start_server_threads();
    // Print startup info
    print_startup_info();
    // Join the server threads
    join_server_threads();

    return EXIT_SUCCESS;
}


void get_input_args(int argc, char* argv[]) {
    int i = 1;  // Start from the first argument (skip the program path)
    port_number = get_input_num(&i, argc, argv, "Please provide TCP port number");
    socket_path = get_input_string(&i, argc, argv, "Please provide UNIX socket path");
}

void exit_handler(void) {
    cinfo("Shutting down...\n");
    cinfo("Stopping server threads...\n");
    pthread_cancel(ping_thread);
    pthread_cancel(requests_thread);
    pthread_mutex_unlock(&connections_mutex);
    cinfo("Disconnecting clients...\n");
    pthread_mutex_lock(&connections_mutex);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].status != EMPTY) remove_connection(&connections[i]);
    }
    cinfo("Closing sockets and event listeners...\n");
    close(epoll_fd);
    shutdown(local_socket_fd, SHUT_RDWR);
    shutdown(network_socket_fd, SHUT_RDWR);
    close(local_socket_fd);
    close(network_socket_fd);
    free_args(1, socket_path);
    unlink(socket_path);
    cinfo("Server was successfully closed. Bye! :)\n");
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
    // Unlink the local server socket path
    unlink(socket_path);
    // Open and initialize the local server socket
    local_socket_fd = catch_perror(socket(AF_UNIX, SOCK_STREAM, 0));
    init_server_socket(local_socket_fd, (struct sockaddr*) &local_server_addr, sizeof local_server_addr);
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
    init_server_socket(network_socket_fd, (struct sockaddr*) &network_server_addr, sizeof network_server_addr);
}

void init_server_socket(int socket_fd, struct sockaddr *addr, socklen_t addr_size) {
    // Bind the server socket address structure to the socket
    catch_perror(bind(socket_fd, addr, addr_size));
    // Start listening requests on the server socket
    catch_perror(listen(socket_fd, MAX_CONNECTIONS));
    // Set up the new connection event (enable new connection requests)
    struct epoll_event epoll_event = { .events = EPOLLIN | EPOLLPRI };
    // Specify the event data format (format which will be received when event is caught)
    event *event = epoll_event.data.ptr = malloc(sizeof *event);
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
        int ret_val = 0;
        usleep(1000 * PING_INTERVAL);
        pthread_mutex_lock(&connections_mutex);
        if (connections_count > 0) ret_val = ping_connections();
        pthread_mutex_unlock(&connections_mutex);
        if (ret_val == -1) exit_error("Something went wrong while pinging clients\n");
        pthread_testcancel();
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
            char* nickname = connections[i].client ? connections[i].client->nickname : UNREGISTERED_NICKNAME;
            cwarn("%s%s%s is not responding\n", C_BRIGHT_MAGENTA, nickname, C_WARN);
            return remove_connection(&connections[i]);
        }
        // Otherwise, ping the current connection
        else return ping_connection(connections[i]);
    }
    return 0;
}

int ping_connection(connection connection) {
    char* nickname = connection.client ? connection.client->nickname : UNREGISTERED_NICKNAME;
    cinfo("Pinging %s%s%s client...\n", C_BRIGHT_MAGENTA, nickname, C_INFO);
    message msg = { .type = PING };
    connection.status = PINGED;
    if (write(connection.client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot ping %s%s%s\n", C_BRIGHT_MAGENTA, nickname, C_ERROR);
        return -1;
    }
    return 0;
}

int remove_connection(connection *conn) {
    // Send the disconnect message to the client
    static message msg = { .type = DISCONNECT };
    if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) return -1;
    // Print info
    client *client = conn->client;
    char* nickname = client ? client->nickname : UNREGISTERED_NICKNAME;
    cinfo("Removing %s%s%s connection...\n", C_BRIGHT_MAGENTA, nickname, C_INFO);
    // Remove connection data
    if (conn == waiting_client_connection) waiting_client_connection = NULL;
    int client_socket_fd = conn->client_socket_fd;
    conn->client = NULL;
    conn->status = EMPTY;
    conn->client_socket_fd = -1;
    // Remove the opponent connection (if exists)
    if (client) {
        connection *opponent_conn = client->opponent_connection;
        // Prevent recursive checking connections of both clients
        client->opponent_connection = NULL;
        if (opponent_conn) {
            free(client->game_state);
            opponent_conn->client->opponent_connection = NULL;
            if (remove_connection(opponent_conn) == -1) return -1;
        }
    }
    // Delete the client connection from the server epoll events
    catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd, NULL));
    // Close the client connection socket
    int ret_val = 0;
    if (close(client_socket_fd) == -1) {
        cperror("Cannot close %s%s%s the connection\n", C_BRIGHT_MAGENTA, nickname, C_ERROR);
        ret_val = -1;
    }
    // Decrement the number of current connections
    connections_count--;
    free(client);
    return ret_val;
}

_Noreturn void* requests_handler(void) {
    // Create an array to store epoll event structs
    struct epoll_event events[MAX_EVENTS_COUNT];
    // Run the requests handling loop
    while (true) {
        int event_count = catch_perror(epoll_wait(epoll_fd, events, MAX_EVENTS_COUNT, -1));
        for (int i = 0; i < event_count; i++) handle_epoll_event(&events[i]);
        pthread_testcancel();
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

void handle_new_connection_request(event *request_event) {
    int client_socket_fd = catch_perror(accept(request_event->data.server_socket_fd, NULL, NULL));
    connection *conn;
    // Inform a client that the new connection could not be established if the server is full
    if (!(conn = create_connection(client_socket_fd))) {
        cwarn("Could not establish the new connection. Server is overloaded\n");
        message msg = { .type = SERVER_FULL };
        // Send a message to the client
        catch_perror(write(client_socket_fd, &msg, sizeof msg));
        // Close the client connection
        catch_perror(close(client_socket_fd));
    // Otherwise, if the server is not full, start listening to the client requests
    } else {
        cinfo("Received a new connection request\n");
        // Create the event struct
        struct epoll_event epoll_event = { .events = EPOLLIN | EPOLLET | EPOLLHUP };
        event *client_event = epoll_event.data.ptr = malloc(sizeof *client_event);
        client_event->type = CLIENT_EVENT;
        client_event->data.connection = conn;  // Associate the new client connection struct with the new event
        // Add this event to the listened events
        catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_fd, &epoll_event));
    }
}

connection* create_connection(int client_socket_fd) {
    connection *conn = NULL;
    pthread_mutex_lock(&connections_mutex);
    conn = find_empty_connection();
    if (conn) {
        conn->client_socket_fd = client_socket_fd;
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

void handle_client_request(event *request_event) {
    connection *conn = request_event->data.connection;
    // Register the client if the client is not registered
    if (conn->status == CONNECTED) {
        register_client(conn);
        return;
    }
    // Otherwise, read the client message
    message msg;
    catch_perror(read(conn->client_socket_fd, &msg, sizeof msg));
    // Handle client request
    switch (msg.type) {
        case PING:  // The client responded to the ping message
            handle_client_ping_msg(conn);
            break;
        case DISCONNECT:  // The client disconnected the game
            handle_client_disconnect_msg(conn);
            break;
        case MOVE:  // The client made a move
            handle_client_move_msg(conn, msg.data);
            break;
        default:
            cwarn("Received unrecognized request message type from %s%s%s\n",
                C_BRIGHT_MAGENTA, conn->client ? conn->client->nickname : UNREGISTERED_NICKNAME, C_WARN
            );
    }
}

void register_client(connection *conn) {
    int ret_val = 0;
    // Read the client nickname from the client socket
    char nickname[MAX_NICKNAME_LENGTH];
    size_t read_length = read(conn->client_socket_fd, nickname, MAX_NICKNAME_LENGTH);
    if (read_length == -1 || read_length > MAX_NICKNAME_LENGTH) {
        cperror("Cannot read the nickname of the registering client\n");
        ret_val = -1;
    } else nickname[read_length] = '\0';

    if (ret_val != -1) {
        // Check if the specified nickname is not already taken
        if (is_nickname_available(nickname)) create_client_struct(conn, nickname);
        // Inform a client that the nickname is already taken and close client connection
        else {
            cwarn("Nickname %s%s%s is already taken. Rejected registration\n", C_BRIGHT_MAGENTA, nickname, C_WARN);
            message msg = { .type = USERNAME_TAKEN };
            if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) {
                cperror("Cannot send a message to the rejected client\n");
            }
            // Set this value to -1 to ensure that the client connection will be removed
            ret_val = -1;
        }
    }
    // Remove the client connection if something went wrong
    if (ret_val == -1) {
        pthread_mutex_lock(&connections_mutex);
        remove_connection(conn);
        pthread_mutex_unlock(&connections_mutex);
    }
}

void create_client_struct(connection *conn, char* nickname) {
    pthread_mutex_lock(&connections_mutex);
    // Allocate memory for a client
    if (!(conn->client = calloc(1, sizeof(client)))) {
        cperror("Cannot allocate memory for the client struct\n");
        remove_connection(conn);
        return;
    }
    // Update the connection status and the client structure
    conn->status = REGISTERED;
    strcpy(conn->client->nickname, nickname);
    cinfo("Client %s%s%s has successfully registered on the server\n", C_BRIGHT_MAGENTA, nickname, C_INFO);
    // Start the game if there is already a client waiting for an opponent
    if (waiting_client_connection) {
        // Remove clients connections if there was an error while starting the game
        if (start_game_between_clients(conn, waiting_client_connection) == -1) {
            cerror("Failed to start the game between %s%s%s and %s%s%s\n",
                   C_BRIGHT_MAGENTA, nickname, C_ERROR,
                   C_BRIGHT_MAGENTA, waiting_client_connection->client->nickname, C_ERROR
            );
            remove_connection(conn);
        }
    }
    // Otherwise, save the current client connection as waiting for an opponent
    else if (wait_for_opponent(conn) == -1) remove_connection(conn);
    pthread_mutex_unlock(&connections_mutex);
}

bool is_nickname_available(char* nickname) {
    if (strcmp(nickname, UNREGISTERED_NICKNAME) == 0) return false;
    // Check if the nickname is already taken
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].client && strcmp(nickname, connections[i].client->nickname) == 0) {
            return false;
        }
    }
    return true;
}

int start_game_between_clients(connection *conn1, connection *conn2) {
    waiting_client_connection = NULL;
    client *client1 = conn1->client;
    client *client2 = conn2->client;
    // Store opponent connections
    client1->opponent_connection = conn2;
    client2->opponent_connection = conn1;
    // Assign random symbols to the clients
    assign_random_symbols(&client1->symbol, &client2->symbol);;
    // Print info
    print_game_update_msg(conn1, "Starting game...\n");
    // Send game start messages to the clients
    if (send_game_start_msg(conn1, client1->symbol) == -1
     || send_game_start_msg(conn2, client2->symbol) == -1) {
        return -1;
    }
    // Create the game state object
    game_state *game_state;
    if (!(game_state = client1->game_state = client2->game_state = calloc(1, sizeof (*game_state)))) {
        cperror("Cannot allocate memory for the game state object\n");
        return -1;
    }
    // Choose the starting player symbol and send the game state to the clients
    game_state->curr_turn_symbol = PLAYER_SYMBOLS[rand() % 2];
    game_state->remaining_moves = 9;
    // A function below sends the game state update to both connected clients
    return send_game_state(conn1);
}

void assign_random_symbols(char *symbol1, char *symbol2) {
    int i = rand() % 2;
    *symbol1 = PLAYER_SYMBOLS[i];
    *symbol2 = PLAYER_SYMBOLS[1 - i];
}

int send_game_start_msg(connection *conn, char symbol) {
    client* client = conn->client;
    // Send the message to the first client
    message msg = { .type = GAME_STARTED, .data.game_start.symbol = symbol };
    strcpy(msg.data.game_start.opponent, client->opponent_connection->client->nickname);
    if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the game start message to %s%s%s\n", C_BRIGHT_MAGENTA, client->nickname, C_ERROR);
        return -1;
    }
    return 0;
}

int wait_for_opponent(connection *conn) {
    waiting_client_connection = conn;
    char* nickname = conn->client->nickname;
    cinfo("Client %s%s%s is waiting for an opponent\n", C_BRIGHT_MAGENTA, nickname, C_INFO);
    // Send the message indicating that the client must wait for an opponent
    message msg = { .type = WAIT_OPPONENT };
    if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the wait message to %s%s%s\n", C_BRIGHT_MAGENTA, nickname, C_ERROR);
        return -1;
    }
    return 0;
}

void handle_client_ping_msg(connection *conn) {
    // Update the connection status to CONNECTED
    pthread_mutex_lock(&connections_mutex);
    char* nickname = conn->client ? conn->client->nickname : UNREGISTERED_NICKNAME;
    cinfo("Client %s%s%s responded\n", C_BRIGHT_MAGENTA, nickname, C_INFO);
    conn->status = conn->client ? REGISTERED : CONNECTED;
    pthread_mutex_unlock(&connections_mutex);
}

void handle_client_disconnect_msg(connection *conn) {
    // Delete the client connection
    pthread_mutex_lock(&connections_mutex);
    remove_connection(conn);
    pthread_mutex_unlock(&connections_mutex);
}

void handle_client_move_msg(connection *conn, msg_data data) {
    // Extract necessary data
    client* curr_client = conn->client;
    client* opponent_client = curr_client->opponent_connection->client;
    game_state *game_state = curr_client->game_state;
    int move_id = data.move_id;

    // Check if the client is allowed to make such a move
    if (is_valid_move(game_state, move_id, curr_client->symbol)) {
        // Update the game state board (shared between both playing clients)
        game_state->board[move_id - 1] = curr_client->symbol;
        game_state->curr_turn_symbol = opponent_client->symbol;
        game_state->remaining_moves--;
        print_game_update_msg(conn, "Registered %s%s%s move\n", C_BRIGHT_MAGENTA, curr_client->nickname, C_BRIGHT_GREEN);
        // Send the current game state
        if (send_game_state(conn) != -1) {
            // If the game result is not equal to PLAYING (the game has finished), send the result to the clients
            game_result game_result = get_game_result(game_state, move_id);
            if (game_result == PLAYING) return;
            send_game_result(conn, game_result);
            print_game_result_msg(conn, game_result);
        }
        // Close clients connection if the game has finished or something went wrong
        pthread_mutex_lock(&connections_mutex);
        remove_connection(conn);
        pthread_mutex_unlock(&connections_mutex);
    }
}

bool is_valid_move(game_state *game_state, int move_id, char client_symbol) {
    // Now is the opponent's turn
    if (game_state->curr_turn_symbol != client_symbol) return false;
    // The client specified wrong move id
    if (move_id < 1 || move_id > 9) return false;
    // Check if the selected field is empty
    return game_state->board[move_id - 1] == '\0';
}

void print_game_update_msg(connection *conn, char* format, ...) {
    va_list args;
    va_start(args, format);
    client *client1 = conn->client;
    client *client2 = client1->opponent_connection->client;
    char* nick1 = client1->nickname;
    char* nick2 = client2->nickname;
    char* COLOR = C_BRIGHT_GREEN;
    char new_format[2 * (strlen(C_BRIGHT_MAGENTA) + strlen(COLOR)) + strlen(nick1) + strlen(nick2) + strlen(format) +  8];
    sprintf(new_format, "[%s%s%s vs %s%s%s] %s", C_BRIGHT_MAGENTA, nick1, COLOR, C_BRIGHT_MAGENTA, nick2, COLOR, format);
    // Print the colorful game state update message
    print_in_color(stdout, COLOR, new_format, args);
}

int send_game_state(connection *conn) {
    print_game_update_msg(conn, "Sending game state update...\n");
    client *client1 = conn->client;
    connection *conn2 = client1->opponent_connection;
    client *client2 = conn2->client;
    message msg = { .type = GAME_STATE };
    // Copy the game state to the message data object
    memcpy(&msg.data.game_state, client1->game_state, sizeof(*client1->game_state));
    // Send the updated game state to the both playing clients
    if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the game state to %s%s%s\n", C_BRIGHT_MAGENTA, client1->nickname, C_ERROR);
        return -1;
    }
    if (write(conn2->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the game state to %s%s%s\n", C_BRIGHT_MAGENTA, client2->nickname, C_ERROR);
        return -1;
    }
    return 0;
}

game_result get_game_result(game_state *game_state, int move_id) {
    const char *board = game_state->board;
    static const int rows[][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
    static const int cols[][3] = {{0, 3, 6}, {1, 4, 7}, {2, 5, 8}};
    static const int tl_br_diag[] = {0, 4, 8};
    static const int tr_bl_diag[] = {2, 4, 6};

    int move_idx = move_id - 1;
    int r = move_idx / 3;
    int c = move_idx % 3;
    char s = board[move_idx];

    // Check if the current player is winning
    if ((board[rows[r][0]] == s && board[rows[r][1]] == s && board[rows[r][2]] == s)
     || (board[cols[c][0]] == s && board[cols[c][1]] == s && board[cols[c][2]] == s)
     || (r == c && board[tl_br_diag[0]] == s && board[tl_br_diag[1]] == s && board[tl_br_diag[2]] == s)
     || (r == 2 - c && board[tr_bl_diag[0]] == s && board[tr_bl_diag[1]] == s && board[tr_bl_diag[2]] == s)) {
        return WON;
    }
    if (game_state->remaining_moves == 0) return DRAW;
    return PLAYING;
}

void print_game_result_msg(connection *conn, game_result game_result) {
    char* nick1 = conn->client->nickname;
    char* nick2 = conn->client->opponent_connection->client->nickname;
    if (game_result == DRAW) print_game_update_msg(conn, "%sDRAW%s\n", C_BRIGHT_YELLOW, C_BRIGHT_GREEN);
    else print_game_update_msg(conn, "%s%s %sWON%s\n", C_BRIGHT_MAGENTA, game_result == WON ? nick1 : nick2, C_BRIGHT_YELLOW, C_BRIGHT_GREEN);
}

int send_game_result(connection *conn, game_result game_result) {
    client *client1 = conn->client;
    connection *conn2 = client1->opponent_connection;
    client *client2 = conn2->client;
    message msg = { .type = GAME_RESULT };
    // Send the game result to the first client
    msg.data.game_result = game_result;
    if (write(conn->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the game state to %s%s%s\n", C_BRIGHT_MAGENTA, client1->nickname, C_ERROR);
        return -1;
    }
    // Send the game result to the opponent client
    msg.data.game_result = get_opposite_game_result(game_result);
    if (write(conn2->client_socket_fd, &msg, sizeof msg) == -1) {
        cperror("Cannot send the game state to %s%s%s\n", C_BRIGHT_MAGENTA, client2->nickname, C_ERROR);
        return -1;
    }
    return 0;
}

game_result get_opposite_game_result(game_result game_result) {
    if (game_result == WON) return LOST;
    if (game_result == LOST) return WON;
    return game_result;
}

void print_startup_info(void) {
    // Print the local server startup info
    cinfo("Local server is listening on %s%s%s\n", C_BRIGHT_MAGENTA, socket_path, C_INFO);
    // Print the network server startup info
    cinfo("Network server is listening on %s*:%d%s\n", C_BRIGHT_MAGENTA, port_number, C_INFO);
}

void join_server_threads(void) {
    // Join the connection thread
    if (pthread_join(ping_thread, NULL) != 0) {
        exit_error("Cannot join the connection thread\n");
    }
    // Join the requests thread
    if (pthread_join(requests_thread, NULL) != 0) {
        exit_error("Cannot join the requests thread\n");
    }
}
