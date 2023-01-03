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

//#define SERVER_FIFO "fifo.pipe" //! Depois de voltar a ler o inunciado percebi que o nome do fifo do servidor é criado pelo utilizador, e depois o utilisador tem de passa-lo para o cliente sempre que o quiser criar

//! Removed the send_msg func because it's only used one time in the whole proj (there are simmililar funcs tha write do different places, but it's not worth having them all in a diferent file from the only place they're used in)

void send_request(int code, char const *session_pipe, char const *box_name) {
    
}