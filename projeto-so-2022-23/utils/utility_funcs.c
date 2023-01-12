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
    char request[2 + MAX_PIPE_NAME + MAX_BOX_NAME + 1]; //code (1-10) | pipe name | box name \0
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





typedef struct node{
    long key;
    char data[DATA_SIZE];
    struct node *next;
}node_l;
typedef node_l *list_l;



//linked list_ls
void insert_input_b_l(list_l *head) {
    //Adiciona uma celula no inicio da list_l de acordo com input
    list_l p;
    p = malloc(sizeof(node_l));
    printf("mensagem\n");
    scanf("%ld",&p->key);
    readNext(p->data);
    p->next = *head;
    *head = p;
}

void insert_input_e_l(list_l *head/*, list_l *ult*/) {
    //Adiciona uma celula no fim da list_l de acordo com o input
    list_l p, q;
    p = *head;
    q = malloc(sizeof(node_l));
    printf("mensagem\n");
    scanf("%#",&q->###);
    q->next = NULL;
    /* *ult->next = q;*/
    while(p->next != NULL)
        p = p->next;
    p->next = q;
}

void insert_b_l(list_l *head, ## ###) {
    //Adiciona uma celula no inicio da list_l
    list_l p;
    p = malloc(sizeof(node_l));
    p->### = ###;
    p->next = *head;
    *head = p;
}

void insert_e_l(list_l *head/*, list_l *ult*/, ## ###) {
    //Adiciona uma celula no fim da list_l
    list_l p, q;
    p = *head;
    q = malloc(sizeof(node_l));
    q->### = ###;
    q->next = NULL;
    /* *ult->next = q;*/
    while(p->next != NULL)
        p = p->next;
    p->next = q;
}

void print_l(list_l *head) {
    //Imheade a list_l completa
    int i;
    list_l p;
    p = *head;
    while(p != NULL){
        printf("%#, %#, %#, %#.\n", p->#, p->#, p->#, p->#);
        p = p->next;
    }
}

list_l search_for_l(list_l *head, ## valproc) {
    //Encontra a celula com o elemento procurado
    list_l p;
    int found = 0;
    p = *head;
    while(found == 0 && p != NULL){
        if(p->### == valproc) 
            found=1;
        else 
            p=p->next;
    }
    if(enc == 1) return p;
    else return NULL;
}

list_l find_l(list_l *head, int index) {
    //Encontra a celula correspondente ao indice
    list_l p;
    p = *head;
    for (i = 1; i <= index; i++){
        p = p->next;
    }
    return p;
}

