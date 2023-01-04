#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logging.h"

/*need to:
create a fifo (char **argv, server fifo)
listen for requests:
    send to producer-consumer -> pick a thread to complete the request
publisher func: read from session fifo and write to the tfs file/box
subscriber func: read from the tfs file/box and write to the session fifo
manager: accomplish request (...)
*/

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "usage: mbroker <pipename>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}