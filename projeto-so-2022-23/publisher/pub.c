#include "../utils/utility_funcs.h"
/*
Based off of named_pipes_sender from the 7th lab
*/

/*need to:
create a fifo (char **argv)             check
send request to server (int argc)   check

format messages
send them to the fifo*/

bool session_end = false;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        session_end = true;
    } else
        exit(EXIT_FAILURE);
}

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   pub <register_pipe> <pipe_name> <box_name>\n");
}

int main(int argc, char **argv) { // TODO check if box already has a publisher
    if (argc != 4) {
        print_usage();
        return 1;
    }
    char *server_pipe = argv[1];
    char *session_pipe = argv[2];
    char *box_name = argv[3];
    //*CREATE SESSION PIPE
    /*The named file already exists.*/

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        exit(EXIT_SUCCESS);

    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST) {
        fprintf(stderr, "Session pipe already exists.\n");
        return 1;
    }

    //*SEND REQUEST TO THE SERVER
    int server_fifo = open(server_pipe, O_WRONLY);
    if (server_fifo == -1) {
        fprintf(stderr, "Open server pipe failed.\n");
        return 1;
    }
    send_request(
        R_PUB, session_pipe, box_name,
        server_fifo); // removed the [...]'s to pass the strings (passing for ex
                      // box_name[9] might not pass the first 9 characters?)

    // open session pipe for writing
    // this waits for server to open it for reading
    int session_fifo = open(session_pipe, O_WRONLY);
    if (session_fifo == -1) {
        fprintf(stderr, "Open session pipe failed.\n");
        close(server_fifo);
        unlink(session_pipe);
        return 1;
    }

    //*WRITE MESSAGE
    size_t len = 1;
    char line[sizeof(uint8_t) +
              MAX_MESSAGE]; //[ code = 9 (uint8_t) ] | [ message (char[1024]) ]
    char message[MAX_MESSAGE];
    while (!session_end) {
        //*PLACE CODE
        uint8_t code = M_PUB;

        //*READS USER INPUT (and detect ctrl D)
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (ferror(stdin))
                ERROR("Failed to read from user input.");
            if (feof(stdin))
                session_end = true;
        }

        len = strlen(message);
        if (len < MAX_MESSAGE)
            memset(message + len - 1, '\0',
                   MAX_MESSAGE - len); //(-1 is to remove the \n)
        Request r = newRequest((uint8_t)code, NULL, NULL, 0, message);

        //*SEND LINE TO SERVER
        ssize_t ret = write(session_fifo, &r, len);
        if (ret < 0)
            ERROR("Write failed.");
    }

    close(session_fifo);
    unlink(session_pipe);
    return 1;
}
