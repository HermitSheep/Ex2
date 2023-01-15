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

void selector(uint8_t code, char *session_pipe, char *box_name);

void codeR_PUB(char *session_pipe, char *box_name);
void codeR_SUB(char *session_pipe, char *box_name);
void codeC_BOX(char *session_pipe, char *box_name);
void codeR_BOX(char *session_pipe, char *box_name);
void codeL_BOX(char *session_pipe);

box *head = NULL;	//For the head of the l list to be visible everywhere

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   mbroker <register_pipe_name> <max_sessions>\n");
}

int main(int argc, char **argv) {
	if (argc != 3) {
        print_usage();
		return 1;
	}
	char *server_pipe = argv[1];
	//char *max_s = argv[2];
	//int max_session = atoi(max_s);

	if (signal(SIGINT, sig_handler) == SIG_ERR) exit(EXIT_SUCCESS);

	char session_pipe[MAX_PIPE_NAME];
	char box_name[MAX_BOX_NAME];
	uint8_t code;

	int tfs_handle = tfs_init(NULL);

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
	tfs_close(tfs_handle);
	close(server_fifo);
	unlink(server_pipe);
    return -1;
}

void selector(uint8_t code, char *session_pipe, char *box_name) {
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
				codeL_BOX(session_pipe);
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
		session_box->n_publishers = 0;
		return;
    }
    
    //*WRITE MESSAGE
	ssize_t ret;
	char line[sizeof(uint8_t) + MAX_MESSAGE + 1];	//[ code = 9 (uint8_t) ] | [ message (char[1024]) ]
	uint8_t code;
    char message[MAX_MESSAGE];
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
	session_box->n_publishers = 0;
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
	if(box_handle == -1) {      //box doesnt exist
        close (session_fifo);
		return;
    }
	//* PRINT MESSAGE
	char line[sizeof(uint8_t) + MAX_MESSAGE + 1];	//[ code = 10 (uint8_t) ] | [ message (char[1024]) ]
	char message[MAX_MESSAGE + 1];
	ssize_t ret;
    bool session_end = false;
	unsigned long int len;
	unsigned long int size = 0;
	while (!session_end) {
		//*READ
		ret = tfs_read(box_handle, message, sizeof(message));
		if (ret == 0);  //*if EOF do nothing
		else ERROR("Server failed in reading from the box");
		len = strlen(message);
		if (len < MAX_MESSAGE) memset(message+len-1, 0, MAX_MESSAGE - len); 

		//*PRINT LINE
		uint8_t code = M_SUB;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, message, sizeof(message));
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
	}
	tfs_close(box_handle);
	close(session_fifo);
}

void codeC_BOX(char *session_pipe, char *box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	unsigned long int size = 0;
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	char line[sizeof(uint8_t) + sizeof(int32_t) + MAX_MESSAGE + 1];	//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	char message[MAX_MESSAGE];
	unsigned long int len;
	ssize_t ret;
	//* CREATE A BOX and verify if that already exist
	box session_box = find_box(head, box_name);
	if (session_box != NULL) {	//box already existed
		strcpy(message, "Caixa já existe.");
		len = strlen(message);
		memset(message+len-1, 0, MAX_MESSAGE - len);
		uint8_t code = R_R_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &(int32_t){-1}, sizeof(int32_t));
		size += sizeof(int32_t);
		memcpy(line + size, message, sizeof(message));
		size += sizeof(message);
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");

		close(session_fifo);
		return;
	}
	else {
		int box_handle = tfs_open(box_name, TFS_O_CREAT);
		int close_ret = tfs_close(box_handle);
		if(box_handle == -1 || close_ret == -1) {      //create box failed
			strcpy(message, "Erro a criar a caixa.");
			len = strlen(message);
			memset(message+len-1, 0, MAX_MESSAGE - len);
			uint8_t code = R_R_BOX;
			memcpy(line + size, &code, sizeof(code));
			size += sizeof(code);
			memcpy(line + size, &(int32_t){-1}, sizeof(int32_t));
			size += sizeof(int32_t);
			memcpy(line + size, message, sizeof(message));
			size += sizeof(message);
			ret = write(session_fifo, line, sizeof(line));
			if (ret < 0)  ERROR("Write failed.");

			close (session_fifo);
			return;
		}
		box  aux = newBox_b(box_name, (uint8_t) 0, BOX_SIZE, (uint64_t) 0, (uint64_t) 0);	//create box succeeded
		insertion_sort(head, aux);

		memset(message, 0, MAX_MESSAGE);		//create box succeeded
		uint8_t code = R_R_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &(int32_t){0}, sizeof(int32_t));
		size += sizeof(int32_t);
		memcpy(line + size, message, sizeof(message));
		size += sizeof(message);
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");

		close(session_fifo);
	}
	ERROR("Something went drastically wrong in server create box.");
}

void codeR_BOX(char *session_pipe,char *box_name){
	char line[sizeof(uint8_t) + sizeof(int32_t) + MAX_MESSAGE + 1];	//[ code = 6 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	char message[MAX_MESSAGE];
	unsigned long int len;
	unsigned long int size = 0;
	ssize_t ret;
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//*Box never existed to begin with
	box session_box = find_box(head, box_name);
	if (session_box == NULL) {	//box doesn't exist to begin with
		strcpy(message, "Caixa não existe.");
		len = strlen(message);
		memset(message+len-1, 0, MAX_MESSAGE - len);
		sprintf(line, "%1" SCNu8 "%2"PRIu32 "%s", R_C_BOX, (int32_t) -1, message);
		uint8_t code = R_R_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &(int32_t){-1}, sizeof(int32_t));
		size += sizeof(int32_t);
		memcpy(line + size, message, sizeof(message));
		size += sizeof(message);
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
		close(session_fifo);
		return;
	}
	//*REMOVE BOX
	ret = tfs_unlink(box_name);
	if (ret == -1) {      //remove box failed
		strcpy(message, "Erro a remover a caixa.");
		len = strlen(message);
		memset(message+len-1, 0, MAX_MESSAGE - len);
		uint8_t code = R_R_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &(int32_t){-1}, sizeof(int32_t));
		size += sizeof(int32_t);
		memcpy(line + size, message, sizeof(message));
		size += sizeof(message);
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
		close (session_fifo);
		return;
	}
	else {	//remove box succeeded
		remove_box(head, box_name);	//removes box from the lit of boxes
		memset(message, 0, MAX_MESSAGE);
		uint8_t code = R_R_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &(int32_t){0}, sizeof(int32_t));
		size += sizeof(int32_t);
		memcpy(line + size, message, sizeof(message));
		size += sizeof(message);
		ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
		close(session_fifo);
		return;
	}
	ERROR("Something went drastically wrong in server remove box.");
}

void codeL_BOX(char *session_pipe){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");

	//*PRINT LINKED LIST
	char line[sizeof(uint8_t)*2 + MAX_BOX_NAME + sizeof(uint64_t)*3 + 1]; //[ code = 8 (uint8_t) ] | [ last (uint8_t) ] | [ box_name (char[32]) ] | [ box_size (uint64_t) ] | [ n_publishers (uint64_t) ] | [ n_subscribers (uint64_t) ]
	box aux = *head;
	uint8_t last = 0;
	unsigned long int size = 0;
	if (aux == NULL) {
		char box_n[MAX_BOX_NAME + 1];
		memset(box_n, 0, sizeof(box_n));
		last = 1;
		uint8_t code = R_L_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &last, sizeof(last));
		size += sizeof(last);
		memcpy(line + size, box_n, sizeof(box_n));
		size += sizeof(box_n);
		memcpy(line + size, &(int64_t){0}, sizeof(int64_t));
		size += sizeof(int64_t);
		memcpy(line + size, &(int64_t){0}, sizeof(int64_t));
		size += sizeof(int64_t);
		memcpy(line + size, &(int64_t){0}, sizeof(int64_t));
		size += sizeof(int64_t);
		ssize_t ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
	}
	while (aux != NULL) {
		if (aux->next == NULL) last = 1;
		uint8_t code = R_L_BOX;
		memcpy(line + size, &code, sizeof(code));
		size += sizeof(code);
		memcpy(line + size, &last, sizeof(last));
		size += sizeof(last);
		memcpy(line + size, &aux->box_name, sizeof(aux->box_name));
		size += sizeof(aux->box_name);
		memcpy(line + size, &aux->box_size, sizeof(aux->box_size));
		size += sizeof(aux->box_size);
		memcpy(line + size, &aux->n_publishers, sizeof(aux->n_publishers));
		size += sizeof(aux->n_publishers);
		memcpy(line + size, &aux->n_subscribers, sizeof(aux->n_subscribers));
		size += sizeof(aux->n_subscribers);

		ssize_t ret = write(session_fifo, line, sizeof(line));
		if (ret < 0)  ERROR("Write failed.");
		aux = aux->next;
	}
	
	close(session_fifo);
}



