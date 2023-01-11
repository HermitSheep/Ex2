#include "../utils/utility_funcs.h"

int main () {
    char code_c[2];
    uint8_t code;
    char instruction[] = "create";
    if (strcmp(instruction, "create"))  code = C_BOX;
    else if (strcmp(instruction, "remove")) code = R_BOX;
    else if (strcmp(instruction, "list")) code = L_BOX;
	printf("code: %d\n", code);

    char line[sizeof(uint8_t) + sizeof(int32_t) + MAX_MESSAGE];	//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	int32_t error_code = 0;
    char error_cod_c[2];
	char error_message[MAX_MESSAGE];
	//*READ
	strcpy(line, "401some made up error.");
	
	//*PRINT LINE
    strncpy(code_c, line, sizeof(code_c));
    code_c[1] = '\0';
    sscanf(code_c, "%hhd", &code);

    strncpy(error_cod_c, line+1, 2);
    error_cod_c[1] = '\0';
    sscanf(error_cod_c, "%"SCNd32, &error_code);

    strncpy(error_message, line+3, sizeof(error_message));
    
    if ((int)error_code == 1)
        fprintf(stdout, "ERROR %s\n", error_message);
    else 
        fprintf(stdout, "OK\n");
}