#ifndef __UTILS_FUNCS_H__
#define __UTILS_FUNCS_H__

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <unistd.h>

#include "logging.h"
#include "operations.h"

#define MAX_PIPE_NAME 256 + 1
#define MAX_BOX_NAME 32 +1
#define MAX_MESSAGE 1024 +1     //for normal and error messages
#define MAX_REQUEST sizeof(uint8_t)+MAX_BOX_NAME+MAX_PIPE_NAME
#define BOX_SIZE (uint64_t) 1024 



typedef enum {		//they're all meant to be unit8_t numbers
	R_PUB = (uint8_t) 1,	//REGISTER PUBLISHER
	R_SUB = (uint8_t) 2,	//REGISTER SUBSCRIBER
	C_BOX = (uint8_t) 3,	//CREATE BOX
	R_C_BOX = (uint8_t) 4,	//RESPONSE TO CREATE BOX
	R_BOX = (uint8_t) 5,	//REMOVE BOX
	R_R_BOX = (uint8_t) 6,	//RESPONSE TO REMOVE BOX
	L_BOX = (uint8_t) 7,	//LIST BOXES
	R_L_BOX = (uint8_t) 8,	//RESPONSE TO LIST BOXES
	M_PUB = (uint8_t) 9, 	//MESSAGE from PUBLISHER to server
	M_SUB = (uint8_t) 10,	//MESSAGE from server to SUBSCRIBER
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


typedef struct request{
	uint8_t code;
	char session_pipe[MAX_PIPE_NAME];
	char box_name[MAX_BOX_NAME];
	int32_t return_code;
	char error_message[MAX_MESSAGE];
	box boxa;

}Request;

box newBox_b(char *name, uint8_t last, uint64_t box_size, uint64_t n_publishers, uint64_t n_subscribers);

Request newRequest(uint8_t code, char *session_pipe_name, char *box_name,int32_t return_code, char *error_message);

void insertion_sort(box* head, box newBox); //function to insert data in sorted position

box find_box(box *head, char* box_name);

bool remove_box(box *head, char* box_name);

void send_request(uint8_t code, char *session_pipe, char *box_name, int rx);

#endif  // __UTILS_FUNCS_H__