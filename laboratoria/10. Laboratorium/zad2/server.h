#ifndef SERVER_H
#define SERVER_H

#define MAX_CONNECTIONS 16
#define UNREGISTERED_NICKNAME "unregistered"

const int PING_INTERVAL = 10000; // ms
const int MAX_EVENTS_COUNT = 16;

// CONNECTION
typedef struct connection connection;

typedef struct client {
    char nickname[MAX_NICKNAME_LENGTH + 1];
    char symbol;
    connection *opponent_connection;
    game_state *game_state;
} client;

typedef enum connection_status {
    EMPTY,       // There is no connection
    REGISTERED,  // A client has successfully registered
    PINGED       // A client was pinged and the server waits for a client's response
} connection_status;

union addr {
    struct sockaddr_un local;
    struct sockaddr_in network;
};

struct connection {
    int client_socket_fd;
    client *client;
    connection_status status;
    socklen_t addr_len;
    union addr addr;
};


// EVENT
typedef enum event_type {
    NEW_CONNECTION,
    CLIENT_EVENT
} event_type;

typedef union event_data {
    int server_socket_fd;
    connection *connection;
} event_data;

typedef struct event {
    event_type type;
    event_data data;
} event;

#endif // SERVER_H
