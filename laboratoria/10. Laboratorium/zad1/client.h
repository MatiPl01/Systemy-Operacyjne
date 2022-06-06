#ifndef CLIENT_H
#define CLIENT_H

const char* LOCAL = "local";
const char* NETWORK = "network";

typedef struct local_state {
    char* nicknames[2];
    char symbols[2];
    struct game_state game;
} local_state;

#endif // CLIENT_H
