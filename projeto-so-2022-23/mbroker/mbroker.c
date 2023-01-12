#include "utility_funcs.h"
#include "producer-consumer.h"

/*need to:
create a fifo (char **argv, server fifo)
listen for requests:
    send to producer-consumer -> pick a thread to complete the request
publisher func: read from session fifo and write to the tfs file/box
subscriber func: read from the tfs file/box and write to the session fifo
manager: accomplish request (...)
*/
bool session_end = false;

static int sig_handler(int sig) {
	if (sig == SIGINT) session_end = true;
	return;
}

int main(int argc, char **argv) {
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	int max_session = argv[2];


	char server_pipe[MAX_PIPE_NAME];
	char box_name[MAX_BOX_NAME];
	uint8_t code;
	tfs_init(NULL);
	char **list_boxs;

	//* CREATE NAMED PIPE - <register_pipe_name>
	/*The named file already exists.*/
	if (mkfifo(server_pipe,0640)== -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//OPEN PIPE IN MODE READ
	int rx = open(server_pipe, O_RDONLY);
	if (rx == -1)  ERROR("Open server pipe failed");
	

	//* PRINT MESSAGE
	size_t len = 1;
	char line[sizeof(uint8_t) + MAX_REQUEST];	//[ code = ? (uint8_t) ] | [ client_named_path (char[256]) | box_name(char[32]) ]
	ssize_t ret;
	while (!session_end) {
		//*READ session_pipe
		ret = read(server_pipe, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;	//? I assume this detects ctrl c too
		else ERROR("Unexpected error while reading");

		//*PRINT LINE
    	sscanf(line, "%2" SCNu8 "%s", &code, server_pipe);
				
		//*READ box_name
		ret = read(server_pipe, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;	//? I assume this detects ctrl c too
		else ERROR("Unexpected error while reading");

	//*PRINT LINE
    	sscanf(line, "%s" , box_name);
	}
	//*box_name not exist
	int bn = open(box_name,O_RDONLY);
	if(bn == -1) ERROR("Open box name failed");


	printf("%s%s%s", server_pipe, server_pipe, box_name);
    return -1;
}