#include "../utils/utility_funcs.h"

void code1(char *session_pipe, char *box_name){
	int tx = open(session_pipe, O_RDONLY);
	if (tx == -1)  ERROR("Open session pipe failed.");
	if(tfs_open(box_name,TFS_O_APPEND) == -1) ERROR("Open box name failed");	//*Check if box exists
	
}