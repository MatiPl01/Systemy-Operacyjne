#include "lib/libcommon.h"


const char* LOCAL = "local";
const char* NETWORK = "network";


int get_input_args(int argc, char* argv[], char** nickname, char** connection_method, char** server_address);
int validate_connection_method(char* connection_method);


int main(int argc, char* argv[]) {
    int exit_status = EXIT_SUCCESS;

    // Get client input arguments
    char* nickname = NULL;
    char* connection_method = NULL;
    char* server_address = NULL;

    if (get_input_args(argc, argv, &nickname, &connection_method, &server_address) == 0) {
        exit_status = EXIT_FAILURE;
        puts("TODO");
    }

    // Release memory
    free_args(3, nickname, connection_method, server_address);

    return exit_status;
}

int get_input_args(int argc, char* argv[], char** nickname, char** connection_method, char** server_address) {
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
        "Please provide server address (IPv4 and port number)" :
        "Please provide server address (path to the UNIX server)"
    );

    return 0;
}

int validate_connection_method(char* connection_method) {
    if (strcmp(connection_method, NETWORK) != 0 || strcmp(connection_method, LOCAL) != 0) {
        print_error("Wrong connection method specified. Shutting down...");
        return -1;
    }
    return 0;
}
