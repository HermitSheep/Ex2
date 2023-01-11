#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logging.h"
#include "operations.h"

/*need to:
Message: format a string to the specifications in "instruções do projeto"/the proj sheet
Finish_session: signal the client and worker thread to sleep (maybe through producer-consumer) and close 
session fifo*/

#define MAX_PIPE_NAME 256
#define MAX_BOX_NAME 32
#define MAX_MESSAGE 1024    //for normal and error messages

void send_request(uint8_t code, char *session_pipe, char *box_name, int rx) {   //rx -> server file indicator
    char zero = '\0'; 
    char request[2 + 1 + MAX_PIPE_NAME + 1 + MAX_BOX_NAME + 1]; //code (1-10) | pipe name | box name \0
    //*BACKFILL NAMES
    if (strlen(session_pipe) <= MAX_PIPE_NAME){
        session_pipe[MAX_PIPE_NAME] = '\0';     //makes sure the max size isn't exceeded
        session_pipe += zero * (MAX_PIPE_NAME - strlen(session_pipe));      //backfills names
    }if (strlen(box_name)< MAX_BOX_NAME){
        session_pipe[MAX_PIPE_NAME] = '\0';
        box_name += zero * (MAX_BOX_NAME - strlen(box_name));
    }
    //*FORMAT REQUEST
    sprintf(request, "%d%s%s", code, session_pipe, box_name);
    //*SEND REQUEST
    if (write(rx, request, sizeof(request)) < 0) ERROR("Failed to send request to server.");
    return;
}       //! we should probably make a read request function too, because all the \0's in the middle of the request make it hard for the reader to know when it's truly finished reading


