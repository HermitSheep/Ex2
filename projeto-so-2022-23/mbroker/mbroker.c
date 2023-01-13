#include "../utils/utility_funcs.h"
#include "producer-consumer.h"
#include "codes.c"

/*need to:
create a fifo (char **argv, server fifo)
listen for requests:
    send to producer-consumer -> pick a thread to complete the request
publisher func: read from session fifo and write to the tfs file/box
subscriber func: read from the tfs file/box and write to the session fifo
manager: accomplish request (...)
*/

box *head = NULL;	//For the head of the l list to be visible everywhere

int main(int argc, char **argv) {
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	int max_session = argv[2];
	signal(SIGINT, sig_handler);

	char session_pipe[MAX_PIPE_NAME];
	char box_name[MAX_BOX_NAME];
	uint8_t code;

	tfs_init(NULL);

	box newB = newBox("a");
	insertion_sort(head, newB);

	//*alocation memory
	size_t capacity = 100;
	pc_queue_t *queue = malloc(sizeof(pc_queue_t));
	
	pcq_create(queue, capacity);

	//* CREATE NAMED PIPE - <register_pipe_name>
	/*The named file already exists.*/
	if (mkfifo(server_pipe,0640)== -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//OPEN PIPE IN MODE READ
	int server_fifo = open(server_pipe, O_RDONLY);
	if (server_fifo == -1)  ERROR("Open server pipe failed");
	

	//* PRINT MESSAGE
	char line[sizeof(uint8_t) + MAX_REQUEST];	//[ code = ? (uint8_t) ] | [ client_named_path (char[256]) | box_name(char[32]) ]
	ssize_t ret;
	bool session_end = false;
	while (!session_end) {
		//*READ session_pipe
		ret = read(server_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;	//? I assume this detects ctrl c too
		else ERROR("Unexpected error while reading");

		//*PRINT LINE
    	sscanf(line, "%2" SCNu8 "%s", &code, session_pipe);
				
		//*READ box_name
		ret = read(server_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;	//? I assume this detects ctrl c too
		else ERROR("Unexpected error while reading");

	//*PRINT LINE
    	sscanf(line, "%s" , box_name);
	}

	free(queue);
    return -1;
}