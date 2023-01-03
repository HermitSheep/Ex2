#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logging.h"
#include "utility_funcs.c"
/*
Based off of named_pipes_sender from the 7th lab
*/

/*need to:
create a fifo (char **argv)             check
send request to server (int argc)   check

format messages
send them to the fifo*/

#define BUFFER_SIZE (128)

int main(char *argc, char **argv, char *box) {  // TODO check if box already has a publisher
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

	// open pipe for writing
	// this waits for someone to open it for reading
	int tx = open(session_pipe, O_WRONLY);
	if (tx == -1) {
		fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//*WRITE MESSAGE
	size_t len = 1;
	char line[256];
	bool end = false;
	while (!end) {
		//*PLACE CODE
		uint8_t code = M_PUB;
		sprintf(line, "%ld|", code);	//?idk if this works

		//*CHECK IF TERMINATION SIGNAL GIVEN
		if (fgets(line[2], sizeof line, stdin) == NULL) end = true;	 //?idk if this works

		//*CLEAR EMPTY SPACE IN LINE
		len = strlen(line) + 2;
		if (len < 256)
			memset(line[len - 1], "\0", 256 - len);  //?idk if this works, but i sure hope it does
			                                         //(-1 is to remove the final \n. I assume only the last \n needs this)

		//*SEND LINE TO SERVER
		ssize_t ret = write(tx, line, len);
		if (ret < 0) {
			fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	close(tx);
	unlink(argv);
}
