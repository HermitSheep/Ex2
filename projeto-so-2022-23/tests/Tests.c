#include "utility_funcs.h"

int main () {
    uint8_t code;
    char instruction[] = "create";
    if (strcmp(instruction, "create"))  code = C_BOX;
    else if (strcmp(instruction, "remove")) code = R_BOX;
    else if (strcmp(instruction, "list")) code = L_BOX;
	printf("code: %d", code);

    char line[1 + 1 + 2 + 1 + MAX_MESSAGE + 1];	//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	char *error_code;
	char *error_message;
	const char *seperator = "|";
    bool session_end = false;
	while (!session_end) {
		//*READ
		strcpy(line, "4|-1|some made up error.");
		
		//*PRINT LINE
		error_code = strtok(line, seperator);
		error_code = strtok(NULL, seperator);
		error_message = strtok(NULL, seperator);
		if (strcmp(error_code, "-1"))
			fprintf(stdout, "ERROR %s\n", error_message);
        else if (error_code != NULL) {
			fprintf(stdout, "OK\n");
            session_end = true;
        }
	}
}