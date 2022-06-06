#include "lib/libcommon.h"
#include "client.h"


int epoll_fd;  // Client's epoll file descriptor
int client_socket_fd = -1;

local_state state;

// Client input arguments
char* nickname;
char* connection_method;
char* server_address;


void get_input_args(int argc, char* argv[]);
void validate_connection_method(void);
void exit_handler(void);
void sigint_handler(int sig_no);
void set_up_server_connection(void);
void connect_to_local_server(void);
void connect_to_network_server(void);
void connect_to_server(int domain, struct sockaddr* addr, socklen_t addr_size);
void set_up_event_handlers(void);
void handle_events(void);
void handle_epoll_event(struct epoll_event *epoll_event);
void handle_stdin_event(void);
void handle_response_event(void);
void handle_server_full_msg(void);
void handle_username_taken_msg(void);
void handle_ping_msg(message *msg);
void handle_wait_opponent_msg(void);
void handle_game_started_msg(game_start *game_start);
void handle_game_state_msg(game_state *game_state);
void handle_game_result_msg(game_result game_result);




#define print(x) write(STDOUT_FILENO, x, sizeof(x))

static const char board[] =
        "\033[0;0H\033[J\033[90m\n"
        "   1 │ 2 │ 3    you\033[0m (%c) %s\n\033[90m"
        "  ───┼───┼───      \033[0m (%c) %s\n\033[90m"
        "   4 │ 5 │ 6 \n"
        "  ───┼───┼───\n"
        "   7 │ 8 │ 9 \n\n\033[0m";

void draw_initial_state() {
    dprintf(STDOUT_FILENO, board,
            state.symbols[0], state.nicknames[0],
            state.symbols[1], state.nicknames[1]);
}

void cell(int x, int y, char* text) {
    dprintf(STDOUT_FILENO, "\033[s\033[%d;%dH%s\033[u", y * 2 + 2, x * 4 + 4, text);
}

void render_input_prompt() {
    if (state.game.curr_turn_symbol != state.symbols[0])
        print("\033[8;0H\033[J\r [ ] \033[33mWait for your move.\r\033[0m\r\033[2C");
    else print("\033[8;0H\033[J\r [ ]\033[J\r\033[2C");
}

void render_update() {
  static char tmp[12];
  for (int x = 0; x < 3; x++)
    for (int y = 0; y < 3; y++) {
      int c = x + y * 3;
      if (state.game.board[c] == '\0') {
        sprintf(tmp, "\033[90m%d\033[0m", c + 1);
        cell(x, y, tmp);
      }

      else {
          char symbol[strlen(C_BLUE) + strlen(C_YELLOW) + strlen(C_RESET) + 2];
          char s = state.game.board[x + y * 3];
          sprintf(symbol, "%s%c%s", s == PLAYER_SYMBOLS[0] ? C_BLUE : C_YELLOW, s, C_RESET);
          cell(x, y, symbol);
      }
    }
  render_input_prompt();
}





int main(int argc, char* argv[]) {
    // Set up the exit handler
    catch_perror(atexit(exit_handler));
    // Set up the SIGINT handler
    catch_perror(set_sa_handler(SIGINT, 0, sigint_handler));
    // Get client input arguments
    get_input_args(argc, argv);
    // Set up the server connection
    set_up_server_connection();
    // Set up event handlers
    set_up_event_handlers();
    // Run the event handling loop
    handle_events();

    return EXIT_SUCCESS;
}


void get_input_args(int argc, char* argv[]) {
    int i = 1;  // Start from the first argument (skip the program path)
    state.nicknames[0] = nickname = get_input_string(&i, argc, argv,
        "Please provide your nickname (displayed name)"
    );
    size_t nick_length = strlen(nickname);
    if (nick_length < MIN_NICKNAME_LENGTH || nick_length > MAX_NICKNAME_LENGTH) {
        cerror("Wrong nickname length. Nickname must have between %s%d%s and %s%d%s signs\n",
            C_BRIGHT_MAGENTA, MIN_NICKNAME_LENGTH, C_ERROR,
            C_BRIGHT_MAGENTA, MAX_NICKNAME_LENGTH, C_ERROR
        );
        exit(EXIT_SUCCESS);
    }

    connection_method = get_input_string(&i, argc, argv,
        "Please choose your connection method.\n"
        "Available methods are listed below:\n"
        "  - network\n"
        "  - local"
    );
    validate_connection_method();

    server_address = get_input_string(&i, argc, argv,
        strcmp(connection_method, NETWORK) == 0 ?
        "Please provide server address (IPv4 and port number) [IPv4:PORT]" :
        "Please provide server address (path to the UNIX server)"
    );
}

void validate_connection_method(void) {
    if (strcmp(connection_method, NETWORK) != 0 && strcmp(connection_method, LOCAL) != 0) {
        cerror("Wrong connection method specified\n");
        exit(EXIT_SUCCESS);
    }
}

void exit_handler(void) {
    cinfo("Shutting down...\n");
    if (client_socket_fd != -1) close(client_socket_fd);
    free_args(3, nickname, connection_method, server_address);
}

void sigint_handler(int sig_no) {
    // Send the disconnect message to the server
    message msg = { .type = DISCONNECT };
    write(client_socket_fd, &msg, sizeof msg);
    cinfo("Received %d signal\n", sig_no);
    exit(EXIT_SUCCESS);
}

void set_up_server_connection(void) {
    // Print info
    cinfo("Setting up the server connection...\n");
    // Create the epoll
    epoll_fd = catch_perror(epoll_create1(0));
    // Connect to the server of choice
    if (strcmp(connection_method, LOCAL) == 0) connect_to_local_server();
    else connect_to_network_server();
}

void connect_to_local_server(void) {  // TODO - fix this type of connection - doesnt work at all
    // Create the local server socket address structure (similarly to the server)
    struct sockaddr_un local_server_addr = { .sun_family = AF_UNIX };
    strcpy(local_server_addr.sun_path, server_address);
    // Open the local server socket and establish connection
    connect_to_server(AF_UNIX, (struct sockaddr*) &local_server_addr, sizeof local_server_addr);
}

void connect_to_network_server(void) {
    // Extract the ipv4 and the port number from the path specified by user
    char* port_str;
    char* ipv4 = strtok_r(server_address, ":", &port_str);
    int port_number = (int) strtol(port_str, NULL, 10);
    // Create the network server socket address structure (similarly to the server)
    struct sockaddr_in network_server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port_number)
    };
    // Validate and save the server ipv4 address
    if (inet_pton(AF_INET, ipv4, &network_server_addr.sin_addr) <= 0) {
        cerror("Invalid ipv4 address\n");
        exit(EXIT_SUCCESS);
    }
    // Open the network server socket and establish connection
    connect_to_server(AF_INET, (struct sockaddr*) &network_server_addr, sizeof network_server_addr);
}

void connect_to_server(int domain, struct sockaddr* addr, socklen_t addr_size) {
    // Open the client socket
    client_socket_fd = catch_perror(socket(domain, SOCK_STREAM, 0));
    // Connect to the server
    if (connect(client_socket_fd, addr, addr_size) == -1) {
        client_socket_fd = -1;
        cerror("Wrong server address specified\n");
        exit(EXIT_SUCCESS);
    }
    // Register the client after establishing connection (write the client nickname)
    catch_perror(write(client_socket_fd, nickname, strlen(nickname)));
}

void set_up_event_handlers(void) {
    // Create the epoll
    epoll_fd = catch_perror(epoll_create1(0));
    // Set up the stdin event handler
    // (this optimizes the board redrawing by doing so only if it is necessary - if there is a pending input event)
    struct epoll_event epoll_stdin_event = {
        .events = EPOLLIN | EPOLLPRI,  // TODO - Check if EPOLLPRI is necessary
        .data.fd = STDIN_FILENO  // Watch the client process standard input - board updated will be sent there
    };
    catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &epoll_stdin_event));
    // Set up the server responses event handler
    struct epoll_event epoll_response_event = {
        .events = EPOLLIN | EPOLLPRI | EPOLLHUP,  // TODO - Check if EPOLLPRI is necessary
        .data.fd = client_socket_fd
    };
    catch_perror(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_fd, &epoll_response_event));
}

void handle_events(void) {
    // Create an array to store epoll event structs (we expect at most 2 events at once - stdin and server response)
    struct epoll_event events[2];
    // Run the event handling loop
    while (true) {
        int event_count = catch_perror(epoll_wait(epoll_fd, events, 2, -1));
        for (int i = 0; i < event_count; i++) handle_epoll_event(&events[i]);
    }
}

void handle_epoll_event(struct epoll_event *epoll_event) {
    // Handle the stdin event
    if (epoll_event->data.fd == STDIN_FILENO) handle_stdin_event();
    // Handle te server response event
    else if (epoll_event->data.fd == client_socket_fd) {
        // Shut down the client process if the connection is hanging up
        if (epoll_event->events & EPOLLHUP) {
            ccritical("Connection to the server has been lost\n");
            exit(EXIT_SUCCESS);
        // Otherwise, handle the server response
        } else handle_response_event();
    }
}

void handle_stdin_event(void) {
    int c;
    // Skip not numerical input
    if (scanf("%d", &c) != 1) {
        int _;
        // Read the whole invalid input from the stdin (empty the stdin)
        while ((_ = getchar()) != EOF && _ != '\n');
        render_input_prompt();
    } else {
        // Reset the input prompt
        render_input_prompt();
        // Check if the input number is correct
        printf("MOVE: %c\n", c);
        if (c < 1 || c > 9 || state.game.board[c - 1] != '\0') return;
        // Otherwise, if the input is correct, send the message to the server
        message msg = { .type = MOVE, .data.move_id = c };
        catch_perror(write(client_socket_fd, &msg, sizeof msg));
    }
}

void handle_response_event(void) {
    // Read the message from the server
    message msg;
    catch_perror(read(client_socket_fd, &msg, sizeof msg));
    // Handle server response
    switch (msg.type) {
        case SERVER_FULL:
            handle_server_full_msg();
            break;
        case USERNAME_TAKEN:
            handle_username_taken_msg();
            break;
        case PING:
            handle_ping_msg(&msg);
            break;
        case WAIT_OPPONENT:
            handle_wait_opponent_msg();
            break;
        case GAME_STARTED:
            handle_game_started_msg(&msg.data.game_start);
            break;
        case GAME_STATE:
            handle_game_state_msg(&msg.data.game_state);
            break;
        case GAME_RESULT:
            handle_game_result_msg(msg.data.game_result);
        default:
            cwarn("Received unrecognized response message type from the server\n");
    }
}

void handle_server_full_msg(void) {
    ccritical("Server is full\n");
    exit(EXIT_SUCCESS);
}

void handle_username_taken_msg(void) {
    cwarn("Nickname %s%s%s is already taken\n", C_BRIGHT_MAGENTA, nickname, C_WARN);
    close(EXIT_SUCCESS);
}

void handle_ping_msg(message *msg) {
    // Respond with the same message (ping the server)
    catch_perror(write(client_socket_fd, msg, sizeof *msg));
}

void handle_wait_opponent_msg(void) {
    cinfo("Waiting for an opponent...\n");
}

void handle_game_started_msg(game_start *game_start) {
    state.nicknames[1] = game_start->opponent;
    state.symbols[0] = game_start->symbol;
    state.symbols[1] = PLAYER_SYMBOLS[0] == game_start->symbol ? PLAYER_SYMBOLS[1] : PLAYER_SYMBOLS[0];
    draw_initial_state();
}

void handle_game_state_msg(game_state *game_state) {
    // Copy the game state to the local state structure
    memcpy(&state.game, game_state, sizeof *game_state);  // TODO - check if free is necessary
    // Refresh the game view
    render_update();
}

void handle_game_result_msg(game_result game_result) {
    switch (game_result) {
        case WON:
            print("\033[8;0H\033[J\r You won ^.^\33[J\n\n");
            break;
        case DRAW:
            print("\033[8;0H\033[J\r It's a draw O_o\33[J\n\n");
            break;
        case LOST:
            print("\033[8;0H\033[J\r You lost T_T\n\n");
            break;
        default:
            return;
    }
    // Exit the game
    exit(EXIT_SUCCESS);
}
