#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vector.h"

const char * del = " ";

Vector* readCommnand() {
	
	char input[2000];
	printf("# ");
	fgets(input, 2000, stdin);
	Vector* command = vector_constructor(8);


	char *token;
	
	/* get the first token */
	token = strtok(input, del);
	

	/* walk through other tokens */
	while( token != NULL ) 
	{
	  
	  vector_appendE(command,token);	
	  token = strtok(NULL, del);
	  
	}
	
	return command;
}


void process (Vector *input)
{

}



int main(int argc, char *argv[]) {
	Vector *input = readCommnand();
	process(input);
}
