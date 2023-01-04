#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logging.h"
#include "utility_funcs.c"

#define BUFFER_SIZE (128)

/*need to:
create a fifo (char **argv)
send request to server (int argc)

listen to the fifo for output
print the output with "fprintf(stdout, "%s\n", message);"*/

int main(int argc, char **argv, char *box_name) {

    //*CREATE SESSION PIPE
    char *session_pipe = argv[1];

	// remove pipe if it does exist //?I don't know if this is what we should do here
    if (unlink(session_pipe) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", session_pipe,
                strerror(errno));
        exit(EXIT_FAILURE);
    }


    /*The named file already exists.*/
    if (mkfifo("olatemporario", 0640) == -1 && errno != EEXIST ){
        fprintf(stderr, "[ERR]: mkfifo is already exist failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // create pipe
    if (mkfifo(session_pipe, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // open pipe for writing
    // this waits for someone to open it for reading
    int tx = open(session_pipe, O_WRONLY);
    if (tx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //*SEND REQUEST TO THE SERVER
    int tx = open(SERVER_FIFO, O_RDONLY);
    if (tx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    send_request(2, session_pipe,box_name);

    //*print MESSAGE
    print_msg( );

	close(tx);
	unlink(argv);
}
