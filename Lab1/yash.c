#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vector.h"

const char * del = " ";
Vector *fileDArr;
Vector *executeArr;
int pipeline;

int status, pid_ch1, pid_ch2, pid;
int pipefd[2];

static void sig_int(int signo) {
  printf("Sending signals to group:%d\n",pid_ch1); // group id is pid of first in pipeline
  kill(-pid_ch1,SIGINT);
}
static void sig_tstp(int signo) {
  printf("Sending SIGTSTP to group:%d\n",pid_ch1); // group id is pid of first in pipeline
  kill(-pid_ch1,SIGTSTP);
}


Vector* readCommnand() {
	
	char input[2000];
	printf("# ");
	fgets(input, 2000, stdin);
	Vector* command = vector_constructor(8);

	char *no_newline = strtok(input, "\n");
	char *token;
	
	/* get the first token */
	token = strtok(no_newline, del);
	

	/* walk through other tokens */
	while( token != NULL ) 
	{
	  
	  vector_appendE(command,token);	
	  token = strtok(NULL, del);
	  
	}
	
	return command;
}


void processCommand (Vector *input)
{
	pipeline = 0;
	Vector* execCommand = vector_constructor(8); 
	Vector *fileD = vector_constructor(3);
	
	for(int i = 0; i < 3; i ++)
	{
		vector_appendE(fileD, NULL);
	}

	for(int i = 0; i < vector_len(input); i++)
	{
		char * token = (char*) vector_get(input, i); 
		
		if(strcmp(token, ">") == 0) 
		{
			
			vector_insertE(fileD, token, 1);

		}
		else if(strcmp(token, "<") == 0)
		{
			vector_insertE(fileD, token, 0);
			
		}
		else if(strcmp(token, "2>") == 0)
		{
			vector_insertE(fileD, token, 2);
			
		}
		else if(strcmp(token, "|") == 0)
		{
			pipeline = 1;
			vector_appendE(executeArr, execCommand);
			vector_appendE(fileDArr, fileD);
			fileD = vector_constructor(3);
			execCommand = vector_constructor(8); 
			for(int i = 0; i < 3; i ++)
			{
				vector_appendE(fileD, NULL);
			}
		}
		else
		{
			vector_appendE(execCommand, token);
		}
	}

	vector_appendE(executeArr, execCommand);
	vector_appendE(fileDArr, fileD);
}

void executeCommand() 
{
	if(pipeline == 1)
	{   
		Vector *execArg = (Vector*) vector_get(executeArr, 0);
		int len = vector_len(execArg);
		char *myargs1[len + 1];
		if(execArg != NULL)
		{		 	
		 	for(int i = 0; i < len; i++)
		 	{
		 		myargs1[i] = (char*) vector_get(execArg, i);
		 	}
			myargs1[len] = NULL;
		}
		else
		{
			fprintf(stderr, "syntax error near unexpected token `|'");
		}
		

		



		execArg = (Vector*) vector_get(executeArr, 1);
		len = vector_len(execArg);
		char *myargs2[len + 1];
		if(execArg != NULL)
		{
			for(int i = 0; i < len; i++)
			{
				myargs2[i] = (char*) vector_get(execArg, i);
			}

			myargs2[len] = NULL;
		}
		else
		{
			fprintf(stderr, "syntax error near unexpected token `|'");
		}







		//Initializing the pipeline

		if (pipe(pipefd) == -1) 
		{
		    perror("pipe");
		    exit(-1);
	  	}

	  	pid_ch1 = fork();
	 	if (pid_ch1 > 0)
	 	{
	    	printf("Child1 pid = %d\n",pid_ch1);
	    	
	    	// Parent
	    	pid_ch2 = fork();
	    	if (pid_ch2 > 0){
	      		printf("Child2 pid = %d\n",pid_ch2);
	      	
		      	if (signal(SIGINT, sig_int) == SIG_ERR)
					printf("signal(SIGINT) error");
		      	
		      	if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
					printf("signal(SIGTSTP) error");
		      	
		      	close(pipefd[0]); //close the pipe in the parent
		      	close(pipefd[1]);
		      	int count = 0;
		      	while (count < 2) 
		      	{
						// Parent's wait processing is based on the sig_ex4.c
					pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
					// wait does not take options:
					//    waitpid(-1,&status,0) is same as wait(&status)
					// with no options waitpid wait only for terminated child processes
					// with options we can specify what other changes in the child's status
					// we can respond to. Here we are saying we want to also know if the child
					// has been stopped (WUNTRACED) or continued (WCONTINUED)
					if (pid == -1) 
					{
					  perror("waitpid");
					  exit(EXIT_FAILURE);
					}
				
					if (WIFEXITED(status)) 
					{
					  printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));
					  count++;
					} 
					else if (WIFSIGNALED(status)) 
					{
					  printf("child %d killed by signal %d\n", pid, WTERMSIG(status));
					  count++;
					} 
					else if (WIFSTOPPED(status)) 
					{
						printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
						printf("Sending CONT to %d\n", pid);
						sleep(4); //sleep for 4 seconds before sending CONT
						kill(pid,SIGCONT);
					}
					else if (WIFCONTINUED(status)) 
					{
						printf("Continuing %d\n",pid);
					}
			    }
		      	return;
	    	}
			else 
			{
				//Child 2
				//sleep(1);
				setpgid(0,pid_ch1); //child2 joins the group whose group id is same as child1's pid
				close(pipefd[1]); // close the write end
				dup2(pipefd[0],STDIN_FILENO);
      			execvp(myargs2[0], myargs2);  // runs word count
			}
		}
	  	else 
	  	{
			// Child 1
			setsid(); // child 1 creates a new session and a new group and becomes leader -
			//   group id is same as his pid: pid_ch1
			close(pipefd[0]); // close the read end
			dup2(pipefd[1],STDOUT_FILENO);  
			execvp(myargs1[0], myargs1);
			

		}
	}
	else
	{
		int pidch1;
		pidch1 = fork();

		if(pidch1 > 0) // parent
		{	
			sleep(1);
		}

		else
		{
			Vector *execArg = (Vector*) vector_get(executeArr, 0);
			if(execArg != NULL)
			{
				
				int len = vector_len(execArg);
				char *myargs[len + 1];
				for(int i = 0; i < len; i++)
				{
					myargs[i] = (char*) vector_get(execArg, i);
				}
				myargs[len] = NULL;
				execvp(myargs[0], myargs);

			}
		}

	}	
}


int main(int argc, char *argv[]) {
	
	while(1)
	{
		fileDArr = vector_constructor(2);
		executeArr  = vector_constructor(0);
		
		Vector *input = readCommnand();
		for(int i = 0; i< vector_len(input); i++)
		{
			printf("%s\n",(char*) vector_get(input, i));
		}
		processCommand(input);
		executeCommand();
	}
}



			// FILE *fp;
			// char * filename = (char*) vector_get(input, i + 1);
			// if ((fp =  fopen(filename, "r")) ==   NULL) 
			// {
			// 	fprintf(stderr,"can't open %s", filename);
			// }
			// else
			// {
			// 	vector_insertE(fileD, fp, 0);
			// }