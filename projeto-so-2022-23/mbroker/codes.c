#include "../utils/utility_funcs.h"

void code1(char *session_pipe, box file){
	int session_fifo = open(session_pipe, O_RDONLY);
	if (session_fifo == -1)  ERROR("Open session pipe failed.");
    
    if (file->n_publishers == 1) {    //there cant be more than one pub to the same box
        close (session_fifo);
		return;
    }

	if(tfs_open(file->box_name, TFS_O_APPEND) == -1) {      //box doesnt 
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
		else if (errno == EINTR && signal(SIGINT, sig_handler) == SIG_ERR) session_end = true;
		else ERROR("Unexpected error while reading");

        //*INTERPRET
    	sscanf(line, "%2" SCNu8 "%s", &code, message);

        //*WRITE TO BOX
    }
}