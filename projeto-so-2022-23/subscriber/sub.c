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

int main(char *argc, char **argv, char *box) {	//server fifo, session fifo, box name
	//*CREATE SESSION PIPE
	char *session_pipe = argv[1];

	// remove pipe if it does exist //?I don't know if this is what we should do here
	if (unlink(session_pipe) != 0 && errno != ENOENT) {
		fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", session_pipe, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// create pipe
	if (mkfifo(session_pipe, 0640) != 0) {
		fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

    //*SEND REQUEST TO THE SERVER
    int rx = open(argv, O_WRONLY);
    if (rx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    send_request(1, session_pipe, box);

	 // open pipe for reading
    // this waits for someone to open it for reading
    int tx = open(session_pipe, O_RDONLY);
    if (tx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	//* PRINT MESSAGE
	ssize_t ret;  // TODO have to add safety for ctrl + C
	char buffer[BUFFER_SIZE];
	while (true) {
		ret = read(rx, buffer, BUFFER_SIZE - 1);
		if (ret == 0) {
			// ret == 0 signals EOF
			fprintf(stderr, "[INFO]: pipe closed\n");
			break;
		} else if (ret == -1) {
			// ret == -1 signals error
			fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		buffer[ret] = 0;
		fputs(buffer, stdout);
	}

	close(tx);
	close(rx);
	unlink(argv);
}
