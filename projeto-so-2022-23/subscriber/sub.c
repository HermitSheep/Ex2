#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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

int main(int argc, char **argv) {
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
	char *box_name = argv[3];
	//*CREATE SESSION PIPE

	/* //* I think the one bellow makes more sense
	// remove pipe if it does exist 
	if (unlink(session_pipe) != 0 && errno != ENOENT) {
		fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", session_pipe, strerror(errno));
		exit(EXIT_FAILURE);
	}
	*/

    /*The named file already exists.*/		//! este talvez faz o mesmo que o acima?
    if (mkfifo(session_pipe, 0640) == -1 && errno != EEXIST ) 
		ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int rx = open(server_pipe, O_WRONLY);
	if (rx == -1) ERROR("Open server pipe failed");

    send_request(R_SUB, session_pipe[MAX_SESSION_PIPE], box_name[MAX_BOX_NAME]);

	// open pipe for reading
	// this waits for someone to open it for reading
	int tx = open(session_pipe, O_RDONLY);
	if (tx == -1)  ERROR("Open session pipe failed.");

	//* PRINT MESSAGE
	size_t len = 1;
	char line[256];
	int received_messages = 0;
	ssize_t ret;
	while (!session_end) {
		//*READ
		ret = read(session_pipe, line, sizeof(line));
		if (ret == 0) session_end = true;  //*detects EOF
		else if (ret == -1) ERROR("Failed to read from user input.");

		//*CTRL+C DETECTION
		if (signal(SIGINT, sig_handler) == SIG_ERR) exit(EXIT_FAILURE);

		//*PRINT LINE
		fprintf(stdout, "%s\n", line);

		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n");

	close(tx);
	close(rx);
	unlink(session_pipe);
}
