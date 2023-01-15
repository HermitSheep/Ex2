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
bool server_running;
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

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		return 1;
	}

	int tfs_handle = tfs_init(NULL);

	//*allocation memory
	size_t capacity = 100;
	pc_queue_t *queue = malloc(sizeof(pc_queue_t));
	
	pcq_create(queue, capacity);

	//* CREATE NAMED PIPE - <register_pipe_name>
	/*The named file already exists.*/
	if (mkfifo(server_pipe,0640)== -1 && errno == EEXIST ) {
		fprintf(stderr, "Create server pipe failed, %s\n", server_pipe);
		return 1;
	}

	//OPEN PIPE IN MODE READ
	int server_fifo = open(server_pipe, O_RDONLY);
	if (server_fifo == -1)  {
		fprintf(stderr, "Open server pipe failed, %s\n", server_pipe);
		unlink(server_pipe);
		return 1;
	}
	

	//* READ REQUEST
	server_running = true;
	ssize_t ret;
	Request request;
	fcntl(server_fifo, F_SETFL, O_NONBLOCK) ;	//to have the read signal if the session pipe was closed
	while (server_running) {
		printf("end? %s\n", (server_running ? "true" : "false"));
		//*READ code and session_pipe

		ret = read(server_fifo, &request, sizeof(request));//[ code = ? (uint8_t) ] | [ client_named_path (char[256]) | box_name(char[32]) ]
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EINTR) {
			fprintf(stderr, "Error wile reading request.\n");
			free(queue);
			tfs_close(tfs_handle);
			close(server_fifo);
			unlink(server_pipe);
			return 1;
		}	//read failed
    	

		printf("code %d, pipe %s, box %s\n", request.code, request.session_pipe, request.box_name);
		selector(request.code, request.session_pipe, request.box_name);
		printf("went to the selector.");
	}
	free(queue);
	tfs_close(tfs_handle);
	close(server_fifo);			//closes so there are no communications conflicts
	unlink(server_pipe);		//delete this type of FIFO pipe
    return -1;
}

//Choose according to the code that appears in the request
void selector(uint8_t code, char *session_pipe, char *box_name) {

	printf("it entered the selector\n");
	switch (code) {
		case R_PUB:		//[ code = 1 (uint8_t) ] 
			codeR_PUB(session_pipe, box_name);
			break;
		case R_SUB:		//[ code = 2 (uint8_t) ] 
			codeR_SUB(session_pipe, box_name);
			break;
		case C_BOX:		//[ code = 3 (uint8_t) ] 
			codeC_BOX(session_pipe, box_name);
			break;
		case R_BOX:		//[ code = 5 (uint8_t) ] 
			codeR_BOX(session_pipe, box_name);
			break;
		case L_BOX:		//[ code = 7 (uint8_t) ] 
			codeL_BOX(session_pipe);
			break;
		default:		//not excepted code
			fprintf(stderr, "Inexistent code selected.\n");
			server_running = false;
			break;
	}
}

//[ code = 1 (uint8_t) ] | [ client_named_pipe_path (char[256]) ] | [ box_name (char[32]) ]

void codeR_PUB(char *session_pipe, char *box_name) {
	int session_fifo = open(session_pipe, O_WRONLY);
	if (session_fifo == -1)  {
			fprintf(stderr, "Failed to open publisher pipe.\n");
			server_running = false;
			return;
		}
	//Checks if this box_name already exists and if it is possible
	box session_box = find_box(head, box_name);
    
	//It is only possible to have one publisher in each box
    if (session_box->n_publishers == 1) {    //there can´t be more than one pub to the same box
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
	while (!session_end) {							//read everything that is in fifo and then close 
		//*READ
		ret = read(session_fifo, line, sizeof(line));
		if (ret == 0);  //*if EOF do nothing
		else if (errno == EAGAIN) {
			session_end = true;
			printf("Session pipe has been closed.");
		}
		else {
			fprintf(stderr, "Unexpected error when reading from session pipe.\n");
			server_running = false;
			return;
		}

        //*INTERPRET
    	sscanf(line, "%2" SCNu8 "%s", &code, message);

        //*WRITE TO BOX
		tfs_write(box_handle, message, strlen(message) + 1);
    }
	tfs_close(box_handle);
	session_box->n_publishers = 0;
	close(session_fifo);
}


//[ code = 2 (uint8_t) ] | [ client_named_pipe_path (char[256]) ] | [ box_name (char[32]) ]

void codeR_SUB(char *session_pipe,char *box_name){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  {
			fprintf(stderr, "Failed to open subscriber pipe.\n");
			server_running = false;
			return;
		}

	//Checks if this box_name already exists and if it is possible
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
	char message[MAX_MESSAGE + 1];
	ssize_t ret;
    bool session_end = false;
	unsigned long int len;
	while (!session_end) {
		//*READ
		ret = tfs_read(box_handle, message, sizeof(message));
		if (ret == 0);  //*if EOF do nothing
		else {
			fprintf(stderr, "Server failed to read from the box.\n");
			server_running = false;
			return;
		}
		len = strlen(message);
		if (len < MAX_MESSAGE) memset(message+len-1, 0, MAX_MESSAGE - len); 

		//*PRINT LINE
		uint8_t code = M_SUB;					
		Request request = newRequest(code, NULL, NULL, 0, message);					//[ code = 10 (uint8_t) ] | [ message (char[1024]) ]
		
		ret = write(session_fifo, &request, sizeof(request));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}
	}
	tfs_close(box_handle);
	close(session_fifo);
}

//[ code = 3 (uint8_t) ] | [ client_named_pipe_path (char[256]) ] | [ box_name (char[32]) ]
void codeC_BOX(char *session_pipe, char *box_name){
	printf("it entered the func\n");
	int session_fifo = open(session_pipe, O_WRONLY);
	if (session_fifo == -1)  {
			fprintf(stderr, "Failed to open manager pipe (create). %s\n", session_pipe);
			server_running = false;
			return;
		}
	
	printf("it's not the fifo\n");

	char message[MAX_MESSAGE];
	unsigned long int len;
	ssize_t ret;

	//* CREATE A BOX and verify if that already exist
	box session_box = find_box(head, box_name);
	printf("it's not the find box\n");
	if (session_box != NULL) {	//box already existed
		strcpy(message, "Caixa já existe.");
		len = strlen(message);
		memset(message+len-1, 0, MAX_MESSAGE - len);
		uint8_t code = R_R_BOX;

		//create a request using struct "Request"
		Request request = newRequest((uint8_t) code, NULL, NULL,-1, message);//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
		printf("it is going to write\n");
		ret = write(session_fifo, &request, sizeof(request));						//write in session_fifo if request id possible or not to be there
		if (ret < 0) {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}

		close(session_fifo);
		return;
	}
	else {
		int box_handle = tfs_open(box_name, TFS_O_CREAT);
		int close_ret = tfs_close(box_handle);
		if(box_handle == -1 || close_ret == -1) {      //create box failed
			strcpy(message, "Erro a criar a caixa.");
			len = strlen(message);
			uint8_t code = R_R_BOX;
			Request request = newRequest((uint8_t) code, NULL, NULL,-1, message);
			memset(message+len-1, 0, MAX_MESSAGE - len);
			ret = write(session_fifo, &request, sizeof(request));
			if (ret < 0)  {
				fprintf(stderr, "Server failed to write to the pipe.\n");
				server_running = false;
				return;
			}

			close (session_fifo);
			return;
		}
		box  aux = newBox_b(box_name, (uint8_t) 0, BOX_SIZE, (uint64_t) 0, (uint64_t) 0);	//create box succeeded
		insertion_sort(head, aux);

		memset(message, 0, MAX_MESSAGE);		//create box succeeded
		Request request = newRequest(M_SUB, NULL, NULL, 0, message);
		ret = write(session_fifo, &request, sizeof(request));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}

		close(session_fifo);
	}
	ERROR("Something went drastically wrong in server create box.");
}


//[ code = 5 (uint8_t) ] | [ client_named_pipe_path (char[256]) ] | [ box_name (char[32]) ]
void codeR_BOX(char *session_pipe,char *box_name){
	char message[MAX_MESSAGE];
	unsigned long int len;
	ssize_t ret;
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  {
			fprintf(stderr, "Failed to open manager pipe (remove).\n");
			server_running = false;
			return;
		}

	box session_box = find_box(head, box_name);
	if (session_box == NULL) {//*Box never existed to begin with
		strcpy(message, "Caixa não existe.");
		len = strlen(message);
		memset(message+len-1, 0, MAX_MESSAGE - len);
		uint8_t code = R_R_BOX;
		Request r = newRequest((uint8_t) code,NULL,NULL,-1, message);		//[ code = 6 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
		ret = write(session_fifo, &r, sizeof(r));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}
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
		Request request = newRequest((uint8_t) code, NULL, NULL,-1, message);
		ret = write(session_fifo, &request, sizeof(request));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}
		close (session_fifo);
		return;
	}
	else {	//remove box succeeded
		remove_box(head, box_name);	//removes box from the lit of boxes
		memset(message, 0, MAX_MESSAGE);
		uint8_t code = R_R_BOX;
		Request request = newRequest((uint8_t) code, NULL, NULL,0, message);
		ret = write(session_fifo, &request, sizeof(request));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}
		close(session_fifo);
		return;
	}
	ERROR("Something went drastically wrong in server remove box.");
}

//[ code = 7 (uint8_t) ] | [ client_named_pipe_path (char[256]) ]
void codeL_BOX(char *session_pipe){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  {
			fprintf(stderr, "Failed to open manager pipe. (list)\n");
			server_running = false;
			return;
		}

	//*PRINT LINKED LIST	
	uint8_t last = 0;
	
	if (head == NULL){
		char box_n[MAX_BOX_NAME + 1];
		memset(box_n, 0, sizeof(box_n));
		last = 1;
		uint8_t code = R_L_BOX;
		box new_box =newBox_b(box_n, 1, BOX_SIZE, 0, 0);
		Request r = newRequest((uint8_t) code,NULL, NULL,0,box_n);//[ code = 8 (uint8_t) ] | [ last (uint8_t) ] | [ box_name (char[32]) ] | [ box_size (uint64_t) ] | [ n_publishers (uint64_t) ] | [ n_subscribers (uint64_t) ]
		r.boxa = new_box;
		ssize_t ret = write(session_fifo, &r, sizeof(r));
		if (ret < 0)  {
			fprintf(stderr, "Server failed to write to the pipe.\n");
			server_running = false;
			return;
		}
	} 
	else {
	box aux = *head;
		while (aux != NULL) {
			if (aux->next == NULL) last = 1;
			uint8_t code = R_L_BOX;
			aux->last =last;
			Request r = newRequest((uint8_t) code,NULL, NULL,0,NULL);
			r.boxa = aux;

			ssize_t ret = write(session_fifo, &r, sizeof(r));
			if (ret < 0)  {
				fprintf(stderr, "Server failed to write to the pipe.\n");
				server_running = false;
				return;
			}
			aux = aux->next;
		}
	}
	
	close(session_fifo);
}



