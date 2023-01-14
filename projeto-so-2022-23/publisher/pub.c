#include "../utils/utility_funcs.h"
/*
Based off of named_pipes_sender from the 7th lab
*/

/*need to:
create a fifo (char **argv)             check
send request to server (int argc)   check

format messages
send them to the fifo*/

int main(int argc, char **argv) {// TODO check if box already has a publisher
	if (argc != 4) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
	char *box_name = argv[3];
	//*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int server_fifo = open(server_pipe, O_WRONLY);
	if (server_fifo == -1)  ERROR("Open server pipe failed");
	send_request(R_PUB, session_pipe, box_name, server_fifo);	//removed the [...]'s to pass the strings (passing for ex box_name[9] might not pass the first 9 characters?)

	// open session pipe for writing
	// this waits for server to open it for reading
	int session_fifo = open(session_pipe, O_WRONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//*WRITE MESSAGE
	size_t len = 1;
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 9 (uint8_t) ] | [ message (char[1024]) ]
	char message[MAX_MESSAGE];
	bool session_end = false;
	while (!session_end) {
		//*PLACE CODE
		uint8_t code = M_PUB;

		//*READS USER INPUT (and detect ctrl D)
		if (fgets(line, sizeof(line), stdin) == NULL) {
			if (ferror(stdin)) ERROR("Failed to read from user input.");
			if (feof(stdin)) session_end = true;
		}

		len = strlen(message);
		if (len < MAX_MESSAGE) memset(message+len-1, '\0', MAX_MESSAGE - len);  //(-1 is to remove the \n)
		memcpy(line + len, code, sizeof(code));
		len += sizeof(code);
		memcpy(line + len,message,sizeof(message));
		len += sizeof(message);




		//*SEND LINE TO SERVER
		ssize_t ret = write(session_fifo, line, len);
		if (ret < 0)  ERROR("Write failed.");
	}

	close(session_fifo);
	unlink(session_pipe);
	return 1;
}
