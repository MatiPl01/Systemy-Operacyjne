#include "common.h"


void thread_cleanup_handler(void* args) {
    // Release thread args memory
    free(args);
}
