#include "../utils/utility_funcs.h"
#include "producer-consumer.h"

/*need to:
create a fifo (char **argv, server fifo)
listen for requests:
    send to producer-consumer -> pick a thread to complete the request
publisher func: read from session fifo and write to the tfs file/box
subscriber func: read from the tfs file/box and write to the session fifo
manager: accomplish request (...)
*/
bool server_running = true;
void sig_handler(int sig) {
	if (sig == SIGINT) {
		server_running = false;
	}
	else exit(EXIT_FAILURE);
}

selector(code, session_pipe, box_name);

void codeR_PUB(session_pipe, box_name);
void codeR_SUB(session_pipe, box_name);
void codeC_BOX(session_pipe, box_name);
void codeR_BOX(session_pipe, box_name);
void codeL_BOX(session_pipe, box_name);

box *head = NULL;	//For the head of the l list to be visible everywhere

int main(int argc, char **argv) {
	if (argc != 3) ERROR("Wrong input. Expected: ./pub <register_pipe> <pipe_name> <box_name>");
	char *server_pipe = argv[1];
	char *max_s = argv[2];
	int max_session = atoi(max_s);

	if (signal(SIGINT, sig_handler) == SIG_ERR) exit(EXIT_SUCCESS);

	char session_pipe[MAX_PIPE_NAME];
	char box_name[MAX_BOX_NAME];
	uint8_t code;

	tfs_init(NULL);

	//*allocation memory
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
	fcntl(server_fifo, F_SETFL, O_NONBLOCK) ;	//to have the read signal if the session pipe was closed
	while (server_running) {
		//*READ code and session_pipe
		ret = read(server_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR) ERROR("Unexpected error while reading");	//read failed

    	sscanf(line, "%2" SCNu8 "%s", &code, session_pipe);

		//*READ box_name
		ret = read(server_fifo, line, sizeof(line));
		if (ret == 0);
		else if (errno == EINTR) ERROR("Unexpected error while reading");

    	sscanf(line, "%s" , box_name);

		selector(code, session_pipe, box_name);
	}

	free(queue);
	close(server_fifo);
	unlink(server_pipe);
    return -1;
}

selector(code, session_pipe, box_name) {
	while(true){
		switch (code) {
			case R_PUB:
				codeR_PUB(session_pipe, box_name);
				break;
			case R_SUB:
				codeR_SUB(session_pipe, box_name);
				break;
			case C_BOX:
				codeC_BOX(session_pipe, box_name);
				break;
			case R_BOX:
				codeR_BOX(session_pipe, box_name);
				break;
			case L_BOX:
				codeL_BOX(session_pipe, box_name);
				break;
			default:
				ERROR("Impossible code was detected in the server pipe.");
		}
	}
}

void codeR_PUB(char *session_pipe, char *box_name) {
	int session_fifo = open(session_pipe, O_WRONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	box session_box = find_box(head, box_name);
    
    if (session_box->n_publishers == 1) {    //there cant be more than one pub to the same box
        close (session_fifo);
		return;
    }
	else session_box->n_publishers = 1;

	int box_handle = tfs_open(session_box->box_name, TFS_O_APPEND);
	if(box_handle == -1) {      //box doesnt 
        close (session_fifo);
		return;
    }
    
    //*WRITE MESSAGE
	ssize_t ret;
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 9 (uint8_t) ] | [ message (char[1024]) ]
	uint8_t code;
    char *message;
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
		else ERROR("Unexpected error while reading");

        //*INTERPRET
    	sscanf(line, "%2" SCNu8 "%s", &code, message);

        //*WRITE TO BOX
		tfs_write(box_handle, message, strlen(message) + 1);
    }
	
	tfs_close(box_handle);
	close(session_fifo);
}

void codeR_SUB(char *session_pipe,char *box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	box session_box = find_box(head, box_name);
    
    if (session_box->n_subscribers <= 0) {    //need subs /sub to the same box
        close (session_fifo);
		return;
    }
	int box_handle = tfs_open(session_box->box_name, TFS_O_APPEND);
	if(box_handle == -1) {      //box doesnt 
        close (session_fifo);
		return;
    }
	//* PRINT MESSAGE
	size_t len = 1;
	char line[sizeof(uint8_t) + MAX_MESSAGE];	//[ code = 10 (uint8_t) ] | [ message (char[1024]) ]
	char *message;
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
		else if (errno == EINTR) ERROR("Unexpected error while reading");	//read failed

		//*PRINT LINE
    	sscanf(line, "%2" SCNu8 "%s", &code, message);
		fprintf(stdout, "%s\n", message);

		received_messages++;
	}

	fprintf(stdout, "%d messages  were received.\n", received_messages);

	close(session_fifo);
	close(session_pipe);
}

void codeC_BOX(char *session_pipe, char *box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");
	typedef BOX *box;

	//* CREATE A BOX and verify if that already exist
	box newBox_b(char *name, uint8_t last, uint64_t box_size, uint64_t n_publishers, uint64_t n_subscribers);


}

void codeR_BOX(char *session_pipe,char *box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//*REMOVE A BOX
	int box = rmdir(box_name);		//function in c that we can remove the name of the directory to be removed
	if (box ==0){
		printf("Box %s remove\n", box_name);
	}
	else{
		printf("Error: Unable to remove box %s\n");
	}
}

void codeL_BOX(char *session_pipe,char * box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//*PRINT LINKED LIST
	struct box_node *head = NULL;
	struct box_node *current = head;
	while (current != NULL){
		printf("%d", current->box_name);
		current = current->next;
	}
	close(session_fifo);
}