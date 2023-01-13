#include "../utils/utility_funcs.h"

/*need to:
create a fifo (char **argv)
send request to server (int argc)

listen to the fifo for output
print the output with "fprintf(stdout, "%s\n", message);"*/

int main(int argc, char **argv) {
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
	char *box_name = argv[3];
	//*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int server_fifo = open(server_pipe, O_WRONLY);
	if (server_fifo == -1) ERROR("Open server pipe failed");
    send_request(R_SUB, session_pipe, box_name, server_fifo);	//removed the [...]'s to pass the strings (passing for ex box_name[9] might not pass the first 9 characters?)

	// open session pipe for reading
	// this waits for server to open it for reading
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//* PRINT MESSAGE
	size_t len = 1;
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 10 (uint8_t) ] | [ message (char[1024]) ]
	char *message;
	int received_messages = 0;
	ssize_t ret;
	uint8_t code;
	bool session_end = false;
	fcntl(session_fifo, F_SETFL, O_NONBLOCK) ;
	while (!session_end) {
		//*READ
		ret = read(session_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EAGAIN) {
			session_end = true;
			printf("Session pipe has been closed.");
		}
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;	//? I assume this detects ctrl c too
		else ERROR("Unexpected error while reading");

		//*PRINT LINE
    	sscanf(line, "%2" SCNu8 "%s", &code, message);
		fprintf(stdout, "%s\n", message);

		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n", received_messages);

	close(session_fifo);
	close(server_fifo);
	unlink(session_pipe);
	return 1;
}
