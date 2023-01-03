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
    int rx = open(argc, O_WRONLY);
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
	size_t len = 1;
	char line[256];
	bool end = false;
	int received_messages = 0;
	while (!end) {
		//*READ
		if (fgets(line, 3, session_pipe) == NULL) {	//ignore first 3 characters (10|)
			if (ferror(session_pipe)) ERROR("Failed to read from user input.");
			end = true;
		}
		if (fgets(line, sizeof(line), session_pipe) == NULL) {  //?idk if this works	(2 because of the code (9) and the |)
			if (ferror(session_pipe)) ERROR("Failed to read from user input.");
			if (feof(session_pipe)) end = true;
		}

		//*PRINT LINE
		fputs(line, stdout);

		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n");
	
	close(tx);
	close(rx);
	unlink(argv);
}
