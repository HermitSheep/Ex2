#include "../utils/utility_funcs.h"

int main () {
    //char code_c[2];
    uint8_t code;
    char instruction[] = "create";
    if (strcmp(instruction, "create"))  code = C_BOX;
    else if (strcmp(instruction, "remove")) code = R_BOX;
    else if (strcmp(instruction, "list")) code = L_BOX;
	printf("code: %d\n", code);

    char line[sizeof(uint8_t) + sizeof(int32_t) + MAX_MESSAGE];	//[ code = 4 (uint8_t) ] | [ return_code (int32_t) ] | [ error_message (char[1024]) ]
	int32_t error_code = 0;
    //char error_cod_c[2];
	char error_message[MAX_MESSAGE];
	//*READ
	strcpy(line, "4-1some made up error.");
	
	//*PRINT LINE
    sscanf(line, "%1" SCNu8 "%2"SCNd32 "%s", &code, &error_code, error_message);

    
    if ((int)error_code == -1)
        fprintf(stdout, "ERROR %s\n", error_message);
    else 
        fprintf(stdout, "OK\n");
}