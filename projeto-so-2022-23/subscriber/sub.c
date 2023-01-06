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
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int rx = open(server_pipe, O_WRONLY);
	if (rx == -1) ERROR("Open server pipe failed");

    send_request(R_SUB, session_pipe, box_name);	//removed the [...]'s to pass the strings (passing for ex box_name[9] might not pass the first 9 characters?)

	// open pipe for reading
	// this waits for someone to open it for reading
	int tx = open(session_pipe, O_RDONLY);
	if (tx == -1)  ERROR("Open session pipe failed.");

	//* PRINT MESSAGE
	size_t len = 1;
	char line[1 + 1 + MAX_PIPE_NAME + 1 + MAX_BOX_NAME];	//[ code = 2 (uint8_t) ] | [ client_named_pipe_path (char[256]) ] | [ box_name (char[32]) ]
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

	fprintf(stdout, "%d messages  were received.\n", received_messages);

	close(tx);
	close(rx);
	unlink(session_pipe);
	return 1;
}
