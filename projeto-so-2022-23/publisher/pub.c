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
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
	char *box_name = argv[3];
	//*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int rx = open(server_pipe, O_WRONLY);
	if (rx == -1)  ERROR("Open server pipe failed");
	send_request(R_PUB, session_pipe, box_name, rx);	//removed the [...]'s to pass the strings (passing for ex box_name[9] might not pass the first 9 characters?)

	// open session pipe for writing
	// this waits for server to open it for reading
	int tx = open(session_pipe, O_WRONLY);
	if (tx == -1)  ERROR("Open session pipe failed.");

	//*WRITE MESSAGE
	size_t len = 1;
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 9 (uint8_t) ] | [ message (char[1024]) ]
	char message[MAX_MESSAGE];
	bool session_end = false;
	while (!session_end) {
		//*PLACE CODE
		uint8_t code = M_PUB;

		//*READS USER INPUT
		ssize_t ret = read(STDIN_FILENO, message, sizeof(message)); //?idk if this works	(2 because of the code (9) and the |)
		if (ret == 0)  session_end = true;  //*detects EOF / CTRL+D
		else if (ret == -1) ERROR("Failed to read from user input.");

		len = strlen(message);	//backfill message with \0
		if (len < MAX_MESSAGE) memset(message[len - 1], '\0', MAX_MESSAGE - len);  //(-1 is to remove the final \n. I assume only the last \n needs this)
		sprintf(line, "%ld%s", code, message);

		//*SEND LINE TO SERVER
		ssize_t ret = write(tx, line, len);
		if (ret < 0)  ERROR("Write failed.");
	}

	close(tx);
	unlink(session_pipe);
	return 1;
}
