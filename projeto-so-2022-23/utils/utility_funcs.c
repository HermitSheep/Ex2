#include "utility_funcs.h"

/*need to:
Message: format a string to the specifications in "instruções do projeto"/the
proj sheet Finish_session: signal the client and worker thread to sleep (maybe
through producer-consumer) and close session fifo*/

box newBox_b(char *name, uint8_t last, uint64_t box_size, uint64_t n_publishers,
             uint64_t n_subscribers) // constructor for newBox
{
    box newBox = (box)malloc(sizeof(BOX));
    strcpy(newBox->box_name, name);
    newBox->last = last;
    newBox->n_publishers = n_publishers;
    newBox->n_subscribers = n_subscribers;
    newBox->box_size = box_size;
    newBox->next = NULL;
    return newBox;
};

Request newRequest(uint8_t code, char *session_pipe_name, char *box_name,
                   int32_t return_code, char *error_message) {
    Request request = {0};
    request.code = code;
    strncpy(request.session_pipe, session_pipe_name, MAX_PIPE_NAME - 1);
    request.session_pipe[MAX_PIPE_NAME - 1] = '\0';
    strncpy(request.box_name, box_name, MAX_BOX_NAME - 1);
    request.box_name[MAX_BOX_NAME - 1] = '\0';
    request.return_code = return_code;
    strncpy(request.error_message, error_message, MAX_MESSAGE - 1);
    request.error_message[MAX_MESSAGE - 1] = '\0';
    return request;
}

void insertion_sort(box *head,
                    box newBox) // function to insert data in sorted position
{
    // If linked list is empty
    if (*head == NULL || strcmp((*head)->box_name, newBox->box_name) >= 0) {
        newBox->next = *head;
        *head = newBox;
        return;
    }

    // Locate the element before insertion
    box current = *head;
    while (current->next != NULL &&
           strcmp(current->next->box_name, newBox->box_name) < 0)
        current = current->next;

    newBox->next = current->next;
    current->next = newBox;
}

box find_box(box *head, char *box_name) {
    // return NULL if don´t find an element wanted
    box aux;
    bool found = false;
    if (head == NULL) {
        return NULL;
    }
    aux = *head;
    while (!found && aux != NULL) {
        if (strcmp(aux->box_name, box_name))
            found = true;
        else
            aux = aux->next;
    }
    if (found)
        return aux;
    else
        return NULL;
}

// Function that removes an element in list of boxes
bool remove_box(box *head, char *box_name) {
    box aux = *head;
    bool found = false;
    bool success = false;
    if (strcmp(aux->box_name, box_name) ==
        0) { // if it's the first element of the list
        *head = aux->next;
        success = true;
        return success;
    }
    while (!found && aux != NULL) { // if it's some other element
        if (strcmp(aux->next->box_name, box_name) == 0) {
            found = true;
            aux->next = aux->next->next;
            success = true;
            return success;
        }
    }
    return success;
}

// Function that help me to verify if the parametrs are correct, if not
// concatenates "\0" to the maximum that the char can have
void send_request(uint8_t code, char *session_pipe, char *box_name,
                  int rx) { // rx -> server file indicator
    //*BACKFILL NAMES
    if (strlen(session_pipe) < MAX_PIPE_NAME) {
        memset(session_pipe + strlen(session_pipe), 0,
               MAX_PIPE_NAME - strlen(session_pipe));
    }
    if (strlen(box_name) < MAX_BOX_NAME) {
        memset(box_name + strlen(box_name), 0, MAX_BOX_NAME - strlen(box_name));
    }

    //*FORMAT REQUEST
    Request request = newRequest(code, session_pipe, box_name, 0,
                                 NULL); // code (1-10) | pipe name | box name \0
    memset(request.error_message, 0, MAX_MESSAGE);

    //*SEND REQUEST
    if (write(rx, &request, sizeof(request)) < 0)
        ERROR("Failed to send request to server.");
    return;
}
