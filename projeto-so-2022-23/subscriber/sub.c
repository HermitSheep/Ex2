#include "../utils/utility_funcs.h"

/*need to:
create a fifo (char **argv)
send request to server (int argc)

listen to the fifo for output
print the output with "fprintf(stdout, "%s\n", message);"*/

bool server_running = false;
void sig_handler(int sig) {
	if (sig == SIGINT) {
		server_running = true;
	}
	else exit(EXIT_FAILURE);
}

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   sub <register_pipe> <pipe_name> <box_name>\n");
}

int main(int argc, char **argv) {
	if (argc != 3){
        print_usage();
		return 1;
	}
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
	char *box_name = argv[3];

	if (signal(SIGINT, sig_handler) == SIG_ERR) exit(EXIT_SUCCESS);

	//*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ){
		fprint(stderr,"Session pipe already exists.\n");
		close(session_pipe);
		unlink(session_pipe);

	}
	//*SEND REQUEST TO THE SERVER
	int server_fifo = open(server_pipe, O_WRONLY);
	if (server_fifo == -1) ERROR("Open server pipe failed");
    send_request(R_SUB, session_pipe, box_name, server_fifo);

	// open session pipe for reading
	// this waits for server to open it for reading
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//* PRINT MESSAGE
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 10 (uint8_t) ] | [ message (char[1024]) ]
	char message[MAX_MESSAGE];
	int received_messages = 0;
	ssize_t ret;
	uint8_t code;
	fcntl(session_fifo, F_SETFL, O_NONBLOCK) ;	//to have the read signal if the session pipe was closed
	while (!server_running) {
		//*READ
		ret = read(session_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EAGAIN) {		//session pipe was closed
			server_running = true;
			printf("Session pipe has been closed.");
		}
		else if (errno == EINTR){
			fprint("Unexpected error while reading.\n");	//read failed
			close(server_fifo);
			close(session_fifo);
			unlink(session_pipe);
		}
		//*PRINT LINE
    	sscanf(line, "%2" SCNu8 "%s", &code, message);

		fprintf(stdout, "%s\n", message);
		//memcpy(line + len, message, sizeof(message));
		
		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n", received_messages);

	close(session_fifo);
	close(server_fifo);
	unlink(session_pipe);
	return 1;
}
