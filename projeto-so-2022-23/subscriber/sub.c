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
#include<signal.h>

#include "logging.h"
#include "utility_funcs.c"

#define BUFFER_SIZE (128)

/*need to:
create a fifo (char **argv)
send request to server (int argc)

listen to the fifo for output
print the output with "fprintf(stdout, "%s\n", message);"*/
bool session_end = false;

static int sig_handler(int sig) {
	if (sig == SIGINT) session_end = true;
	return;
}

int main(char *argc, char **argv, char *box_name) {	//server fifo, session fifo, box name
	//*CREATE SESSION PIPE
	char *session_pipe = argv[1];

	// remove pipe if it does exist //?I don't know if this is what we should do here
	if (unlink(session_pipe) != 0 && errno != ENOENT) {
		fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", session_pipe, strerror(errno));
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
    int rx = open(argc, O_WRONLY);
    if (rx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    send_request(R_SUB, session_pipe[MAX_SESSION_PIPE], box_name[MAX_BOX_NAME]);

	 // open pipe for reading
    // this waits for someone to open it for reading
    int tx = open(session_pipe, O_RDONLY);
    if (tx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	//* PRINT MESSAGE
	size_t len = 1;
	char line[256];
	int received_messages = 0;
	ssize_t ret;
	while (!session_end) {
		//*READ
		ret = read(session_pipe, line, sizeof(line));
		if (ret == 0) {	//ignore first 3 characters (10|)
            // ret == 0 signals EOF
            fprintf(stderr, "[INFO]: pipe closed\n");
            break;
		}

		//*CTRL+C DETECTION
		if (signal(SIGINT, sig_handler) == SIG_ERR) exit(EXIT_FAILURE);

		//*PRINT LINE
		fprintf(stdout, "%s\n", line);

		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n");

	close(tx);
	close(rx);
	unlink(argv);
}
