#ifndef __UTILS_FUNCS_H__
#define __UTILS_FUNCS_H__

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
#include <inttypes.h>
#include <unistd.h>

#include "logging.h"
#include "operations.h"

#define MAX_PIPE_NAME 256
#define MAX_BOX_NAME 32
#define MAX_MESSAGE 1024    //for normal and error messages
#define MAX_REQUEST 256+32+1


typedef enum {		//they're all meant to be unit8_t numbers
	R_PUB = 1,	//REGISTER PUBLISHER
	R_SUB = 2,	//REGISTER SUBSCRIBER
	C_BOX = 3,	//CREATE BOX
	R_C_BOX = 4,	//RESPONSE TO CREATE BOX
	R_BOX = 5,	//REMOVE BOX
	R_D_BOX = 6,	//RESPONSE TO DELETE BOX
	L_BOX = 7,	//LIST BOXES
	R_L_BOX = 8,	//RESPONSE TO LIST BOXES
	M_PUB = 9, 	//MESSAGE from PUBLISHER to server
	M_SUB = 10,	//MESSAGE from server to SUBSCRIBER
} codes;


typedef enum type{
	pub = 0,
	sub = 1,
} type_client;

typedef struct box{
	int n_publicher;
	char bname;
} box;

typedef struct Element{
    int data;
    struct Element* next;

};




struct Element* newElement(int data)
{
	struct Element* newElement= (struct Element*)malloc(sizeof(struct Element));
	newElement->data = data;
	newElement->next = NULL;
	return newElement;
};

void insertion_sort(struct Element** head, struct Element* newElement); //function to insert data in sorted position

void send_request(uint8_t code, char *session_pipe, char *box_name, int rx);

#endif  // __UTILS_FUNCS_H__