#include "libcommon.h"


void thread_cleanup_handler(void* args) {
    // Release thread args memory
    free(args);
}
