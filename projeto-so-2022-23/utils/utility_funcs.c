#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*need to:
Message: format a string to the specifications in "instruções do projeto"/the proj sheet
Finish_session: signal the client and worker thread to sleep (maybe through producer-consumer) and close 
session fifo*/

#define SERVER_FIFO "fifo.pipe"
#define MAX_SESSION_PIPE 256
#define MAX_BOX_NAME 32
#define MAX_ERROR_MESSAGE 1024

// Helper function to send messages
// Retries to send whatever was not sent in the beginning
void send_msg(int tx, char const *str) {
    size_t len = strlen(str);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(tx, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += ret;
    }
}

void send_request(int code, char const *session_pipe, char const *box_name) {
    char zero = "0"; 
    if (strlen(session_pipe) < MAX_SESSION_PIPE){
        session_pipe += zero * (MAX_SESSION_PIPE - strlen(session_pipe));
    }
    if (strlen(box_name)< MAX_BOX_NAME){
        box_name += zero * (MAX_BOX_NAME - strlen(box_name));
    }

}

