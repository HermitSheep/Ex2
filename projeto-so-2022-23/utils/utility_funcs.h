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
#define BOX_SIZE 1024 



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


typedef struct box_node{	//inside name
	char box_name[MAX_BOX_NAME];
	uint8_t last;
	uint64_t box_size;
	uint64_t n_publishers;
	uint64_t n_subscribers;
	struct box_node *next;
}BOX;	//outside name
typedef BOX *box;	//no need to keep adding *'s

box newBox_b(char *name, uint8_t last, uint64_t box_size, uint64_t n_publishers, uint64_t n_subscribers)
{
	box newBox= (box)malloc(sizeof(BOX));
	strcpy(newBox->box_name, name);
	newBox->last = last;
	newBox->n_publishers = n_publishers;
	newBox->n_subscribers = n_subscribers;
	newBox->box_size = box_size;
	newBox->next = NULL;
	return newBox;
};

void insertion_sort(box* head, box newBox); //function to insert data in sorted position

box find_box(box *head, char* box_name);

void send_request(uint8_t code, char *session_pipe, char *box_name, int rx);

#endif  // __UTILS_FUNCS_H__