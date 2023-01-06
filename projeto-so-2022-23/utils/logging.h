#ifndef __UTILS_LOGGING_H__
#define __UTILS_LOGGING_H__

#include <stdio.h>
#include <stdlib.h>

typedef enum {
	LOG_QUIET = 0,
	LOG_NORMAL = 1,
	LOG_VERBOSE = 2,
} log_level_t;

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

void set_log_level(log_level_t level);
extern log_level_t g_level;

/*debugging tools*/

//prints a message and the position in the program
#define INFO(...)                                                                           \
	do {                                                                                    \
		char buf[2048];                                                                     \
		snprintf(buf, 2048, __VA_ARGS__);                                                   \
		fprintf(stderr, "[INFO]:  %s:%d :: %s :: %s\n", __FILE__, __LINE__, __func__, buf); \
	} while (0);

//prints an error message and the position in the program
#define ERROR(...)                                                                           \
	do {                                                                                    \
		char buf[2048];                                                                     \
		snprintf(buf, 2048, __VA_ARGS__);                                                   \
		fprintf(stderr, "[ERROR]:  %s:%d :: %s :: %s :: %s\n", __FILE__, __LINE__, __func__, strerror(errno), buf); \
		exit(EXIT_FAILURE); \
	} while (0);

//prints the position on the program and quits it
#define PANIC(...)                                                                          \
	do {                                                                                    \
		char buf[2048];                                                                     \
		snprintf(buf, 2048, __VA_ARGS__);                                                   \
		fprintf(stderr, "[PANIC]: %s:%d :: %s :: %s\n", __FILE__, __LINE__, __func__, buf); \
		exit(EXIT_FAILURE);                                                                 \
	} while (0);

//prints a message in error format
#define WARN(...)                                                                               \
	do {                                                                                        \
		if (g_level == LOG_NORMAL || g_level == LOG_VERBOSE) {                                  \
			char buf[2048];                                                                     \
			snprintf(buf, 2048, __VA_ARGS__);                                                   \
			fprintf(stderr, "[WARN]:  %s:%d :: %s :: %s\n", __FILE__, __LINE__, __func__, buf); \
		}                                                                                       \
	} while (0);

//prints a message in log format
#define LOG(...)                                                                                \
	do {                                                                                        \
		if (g_level == LOG_NORMAL || g_level == LOG_VERBOSE) {                                  \
			char buf[2048];                                                                     \
			snprintf(buf, 2048, __VA_ARGS__);                                                   \
			fprintf(stderr, "[LOG]:   %s:%d :: %s :: %s\n", __FILE__, __LINE__, __func__, buf); \
		}                                                                                       \
	} while (0);

//prints a message in debug format
#define DEBUG(...)                                                                              \
	do {                                                                                        \
		if (g_level == LOG_VERBOSE) {                                                           \
			char buf[2048];                                                                     \
			snprintf(buf, 2048, __VA_ARGS__);                                                   \
			fprintf(stderr, "[DEBUG]: %s:%d :: %s :: %s\n", __FILE__, __LINE__, __func__, buf); \
		}                                                                                       \
	} while (0);

//checks for a function failure
#define CHECK(X)                                                                           \
	({                                                                                     \
		int __val = (X);                                                                   \
		(__val == -1 ? ({                                                                  \
			fprintf(stderr, "[ERROR] (" __FILE__ ":%d) -- %s\n", __LINE__, strerror(errno)); \
			exit(-1);                                                                      \
			-1;                                                                            \
		})                                                                                 \
		             : __val);                                                             \
	})

#endif  // __UTILS_LOGGING_H__
