#ifndef CLIENT_H
#define CLIENT_H

const char* LOCAL = "local";
const char* NETWORK = "network";
char* PLAYER_COLORS[] = {
    C_BRIGHT_BLUE,
    C_GREEN
};

typedef struct local_state {
    char* nicknames[2];
    char symbols[2];
    struct game_state game;
} local_state;

static const char board[] =
        "\033[0;0H\033[J\033[90m\n"
        "   1 │ 2 │ 3    you\033[0m (%s) %s\n\033[90m"
        "  ───┼───┼───      \033[0m (%s) %s\n\033[90m"
        "   4 │ 5 │ 6 \n"
        "  ───┼───┼───\n"
        "   7 │ 8 │ 9 \n\n\033[0m";

#endif // CLIENT_H
