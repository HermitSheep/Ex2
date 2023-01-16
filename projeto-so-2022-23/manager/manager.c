#include "../utils/utility_funcs.h"

bool session_end = false;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        session_end = true;
    } else
        exit(EXIT_FAILURE);
}

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 5) { // verify the only char can be there
        print_usage();
        printf("%d", argc);
        return 1;
    }
    char *server_pipe = argv[1];
    char *session_pipe = argv[2];
    char *instruction = argv[3];
    char *box_name = argv[4];

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        exit(EXIT_SUCCESS); // for control-C

    //*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST) {
        fprintf(stderr,
                "Create session pipe failed, server %s session %s instruction "
                "%s box %s\n",
                server_pipe, session_pipe, instruction, box_name);
        return 1;
    }

    //*SEND REQUEST TO THE SERVER
    int server_fifo = open(server_pipe, O_WRONLY);
    if (server_fifo == -1) {
        fprintf(stderr, "Open server pipe failed, %s\n", server_pipe);
        unlink(session_pipe);
        return 1;
    }
    uint8_t code;
    if (strcmp(instruction, "create") == 0)
        code = C_BOX;
    else if (strcmp(instruction, "remove") == 0)
        code = R_BOX;
    else if (strcmp(instruction, "list") == 0)
        code = L_BOX;

    send_request(code, session_pipe, box_name, server_fifo);

    printf("%d\n", code);

    // open pipe for writing
    // this waits for someone to open it for reading
    int session_fifo = open(session_pipe, O_RDONLY);
    if (session_fifo == -1) {
        fprintf(stderr, "Open session pipe failed, %s\n", session_pipe);
        close(server_fifo);
        unlink(session_pipe);
        return 1;
    }

    //* PRINT MESSAGE
    char line[sizeof(uint8_t) + sizeof(int32_t) +
              MAX_MESSAGE]; //[ code = 4 (uint8_t) ] | [ return_code (int32_t) ]
                            //| [ error_message (char[1024]) ]
    ssize_t ret;

    int32_t error_code;
    char error_message[MAX_MESSAGE];

    uint8_t last = 0;

    bool print = false; // if we need to print the list of boxes
    box *head = NULL;
    box aux;
    box aux2;
    Request request;

    while (!session_end) {

        //*READ (doesn't check for pipe closure because that's not a normal
        //behavior here)

        ret = read(session_fifo, &request, sizeof(request));
        if (ret == -1) {
            fprintf(stderr, "Read from session pipe failed.\n");
            close(server_fifo);
            close(session_fifo);
            unlink(session_pipe);
            return 1;
        }

        code = request.code;
        error_code = request.return_code;
        strcpy(error_message, request.error_message);

        //*PRINT LINE
        sscanf(line, "%1" SCNu8, &code);
        if ((int)code == 4 ||
            (int)code == 6) { // response from box creation/deletion
            if ((int)error_code == -1)
                fprintf(stdout, "ERROR %s\n", error_message);
            else
                fprintf(stdout, "OK\n");
            session_end = true;

        } else { // has to list all boxes
            aux = newBox_b(request.boxa->box_name, request.boxa->last,
                           request.boxa->box_size, request.boxa->n_publishers,
                           request.boxa->n_subscribers);
            insertion_sort(head, aux);
            if (last == 1) {
                if (strlen(box_name) == 0) {
                    fprintf(stdout, "NO BOXES FOUND\n");
                    close(session_fifo);
                    close(server_fifo);
                    unlink(session_pipe);
                    return 1;
                }
                session_end = true;
                print = true;
            }
        }
    }
    if (print && head != NULL) { //&& head != NULL is checked so the compiler
                                 //doesnt sense an error
        aux = *head;
        aux2 = aux;
        do {
            fprintf(stdout, "%s %zu %zu %zu\n", aux->box_name, aux->box_size,
                    aux->n_publishers, aux->n_subscribers);
            aux = aux->next;
            free(aux2);
            aux2 = aux;
        } while (aux->next == NULL);
    } else {
        fprintf(stderr, "Box creation somehow failed.\n");
        close(server_fifo);
        close(session_fifo);
        unlink(session_pipe);
        return 1;
    }

    close(session_fifo);
    close(server_fifo);
    unlink(session_pipe);
    return 0;
}
