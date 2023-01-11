#include "../utils/utility_funcs.h"

static void print_usage() {
    fprintf(stderr, "usage: \n"
                    "   manager <register_pipe_name> create <box_name>\n"
                    "   manager <register_pipe_name> remove <box_name>\n"
                    "   manager <register_pipe_name> list\n");
}

int main(int argc, char **argv) {
	if (argc <3 || argc > 4){
        print_usage();
        ERROR("Wrong input.");
    }
	char *server_pipe = argv[1];
	char *session_pipe = argv[2];
    char *instruction = argv[3];
	char *box_name = argv[4];

    //*CREATE SESSION PIPE
    /*The named file already exists.*/
    if (mkfifo(session_pipe, 0640) == -1 && errno == EEXIST ) ERROR("Session pipe already exists.");

	//*SEND REQUEST TO THE SERVER
	int rx = open(server_pipe, O_WRONLY);
	if (rx == -1)  ERROR("Open server pipe failed");
    uint8_t code;
    if (strcmp(instruction, "create"))  code = C_BOX;
    else if (strcmp(instruction, "remove")) code = R_BOX;
    else if (strcmp(instruction, "list")) code = L_BOX;
	send_request(code, session_pipe, box_name, rx);

	// open pipe for writing
	// this waits for someone to open it for reading
	int tx = open(session_pipe, O_RDONLY);
	if (tx == -1)  ERROR("Open session pipe failed.");

    //* PRINT MESSAGE
	char line[sizeof(uint8_t) + sizeof(int32_t) + MAX_MESSAGE];	//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	ssize_t ret;
	int32_t error_code;
	char *error_message;
    bool session_end = false;
	while (!session_end) {
		//*READ
		ret = read(session_pipe, line, sizeof(line));
		if (ret == -1) ERROR("Failed to read from user input.");
		
		//*PRINT LINE
		sscanf(line, "%d%ld%s", &code, error_code, error_message);
		if (strcmp(error_code, "-1"))
			fprintf(stdout, "ERROR %s\n", error_message);
        else if (error_code != NULL) {
			fprintf(stdout, "OK\n");
            session_end = true;
        }
	}
    return 1;
}
