#include "utility_funcs.h"

/*need to:
Message: format a string to the specifications in "instruções do projeto"/the proj sheet
Finish_session: signal the client and worker thread to sleep (maybe through producer-consumer) and close 
session fifo*/
void insertion_sort(box* head, box newBox)//function to insert data in sorted position
{
	//If linked list is empty
	if (*head == NULL || strcmp((*head)->box_name, newBox->box_name) >= 0)
	{
		newBox->next = *head;
		*head = newBox;
		return;
	}

	//Locate the element before insertion
	box current = *head;
	while (current->next != NULL && strcmp(current->next->box_name, newBox->box_name) < 0)
		current = current->next;

	newBox->next = current->next;
	current->next = newBox;
}

box find_box(box *head, char* box_name) {
    //Encontra a celula com o elemento procurado
    box aux;
    bool found = false;
    aux = *head;
    while(!found && aux != NULL){
        if(strcmp(aux->box_name, box_name)) 
            found=true;
        else 
            aux=aux->next;
    }
    if(found) return aux;
    else return NULL;
}

bool remove_box(box *head, char* box_name) {
    box aux;
    bool found = false;
    bool success = false;
    aux = *head;
    if (strcmp(aux->box_name, box_name) == 0) { //if it's the first element of the list
        head = aux->next;
        success = true;
        return success;
    }
    while(!found && aux != NULL) {      //if it's some other element
        if(strcmp(aux->next->box_name, box_name) == 0) {
            found=true;
            aux->next = aux->next->next;
            success = true;
            return success;
        }
    }
    return success;
}

void send_request(uint8_t code, char *session_pipe, char *box_name, int rx) {   //rx -> server file indicator
    char zero = 0; 
    char request[2 + MAX_REQUEST]; //code (1-10) | pipe name | box name \0
    //*BACKFILL NAMES
    if (strlen(session_pipe) <= MAX_PIPE_NAME){
        session_pipe += zero * (MAX_PIPE_NAME - strlen(session_pipe));      //backfills names
    }if (strlen(box_name) < MAX_BOX_NAME){
        box_name += zero * (MAX_BOX_NAME - strlen(box_name));
    }
    //*FORMAT REQUEST
    sprintf(request, "%d%s%s", code, session_pipe, box_name);
    //*SEND REQUEST
    if (write(rx, request, sizeof(request)) < 0) ERROR("Failed to send request to server.");
    return;
}
