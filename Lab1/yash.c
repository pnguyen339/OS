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

int totalCom = 0;
Vector *pidJobs;
int pidNum[100];
int finished[100];
int current;
char *current_command;
int bgPogress;


int status, pid_ch1, pid_ch2, pid;
int pipefd[2];

static void sig_int(int signo) 
{
  kill(-pid_ch1,SIGINT);
  int i;
	for(i = 0; i < totalCom; i++)
	{
		if(pid_ch1 == pidNum[i])
		{
			finished[i] = 1;
			break;
		}
	}
}
static void sig_tstp(int signo)
{
  	printf("shit");
  	kill(-pid_ch1,SIGTSTP);
	int i;
	for(i = 0; i < totalCom; i++)
	{
		if(pid_ch1 == pidNum[i])
		{
			finished[i] = 2;
			break;
		}
	}
	signal(SIGTTOU, SIG_IGN);
	tcsetpgrp(0, pid_ch1);
	
}



// Vector* readCommnand() {
	
// 	char input[2000];
// 	printf("# ");
// 	fgets(input, 2000, stdin);
// 	Vector* command = vector_constructor(8);

// 	char *no_newline = strtok(input, "\n");
// 	char *token;
	
// 	 get the first token 
// 	token = strtok(no_newline, del);
	

// 	/* walk through other tokens */
// 	while( token != NULL ) 
// 	{
	  
// 	  vector_appendE(command,token);	
// 	  token = strtok(NULL, del);
	  
// 	}
	
// 	return command;
// }


int processCommand (Vector *input)
{
	int notJobControl = 0;
	pipeline = 0;
	bgPogress = 0;
	Vector *execCommand = vector_constructor(8); 
	Vector *fileD = vector_constructor(3);
	
	for(int i = 0; i < 3; i ++)
	{
		vector_appendE(fileD, NULL);
	}

	for(int i = 0; i < vector_len(input); i++)
	{
		char * token = (char*) vector_get(input, i); 
		
		
		if(strcmp(token, "jobs") == 0 && notJobControl == 0)
		{
			vector_delete(execCommand);
			vector_delete(fileD);
			return 1;
		}
		else if(strcmp(token, "fg") == 0 && notJobControl == 0)
		{
			vector_delete(execCommand);
			vector_delete(fileD);
			return 2;
		}
		else if(strcmp(token, "bg") == 0 && notJobControl == 0)
		{
			vector_delete(execCommand);
			vector_delete(fileD);
			return 3;
		}
		else if(strcmp(token, ">") == 0) 
		{
			notJobControl = 1;
			token = (char*) vector_get(input, i + 1); 
			vector_insertE(fileD, token, 1);
			i++;

		}
		else if(strcmp(token, "<") == 0)
		{
			notJobControl = 1;
			token = (char*) vector_get(input, i + 1); 
			vector_insertE(fileD, token, 0);
			i++;
			
		}
		else if(strcmp(token, "2>") == 0)
		{
			notJobControl = 1;
			token = (char*) vector_get(input, i + 1); 
			vector_insertE(fileD, token, 2);
			i++;
			
		}
		else if(strcmp(token, "|") == 0)
		{
			notJobControl = 1;
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
		else if(strcmp(token, "&") == 0)
		{
			bgPogress = 1;
		}
		else
		{
			notJobControl = 1;
			vector_appendE(execCommand, token);
		}
	}

	vector_appendE(executeArr, execCommand);
	vector_appendE(fileDArr, fileD);
	return 0;
}

void executeCommand() 
{
	
	if (signal(SIGINT, sig_int) == SIG_ERR)
		printf("signal(SIGINT) error");
		      	
	
	

	if(pipeline == 1)
	{   
		Vector *execArg = (Vector*) vector_get(executeArr, 0);
		Vector *filRedirection1 = (Vector*) vector_get(fileDArr, 0);
		int len = vector_len(execArg);
		char *myargs1[len + 1];
		if(execArg != NULL)
		{		 	
		 	for(int i = 0; i < len; i++)
		 	{
		 		myargs1[i] = strdup((char*) vector_get(execArg, i));
		 	}
			myargs1[len] = NULL;
		}
		else
		{
			fprintf(stderr, "syntax error near unexpected token `|'");
		}
		
		vector_delete(execArg);
		



		execArg = (Vector*) vector_get(executeArr, 1);
		Vector *filRedirection2 = (Vector*) vector_get(fileDArr, 1);
		len = vector_len(execArg);
		char *myargs2[len + 1];
		if(execArg != NULL)
		{
			for(int i = 0; i < len; i++)
			{
				myargs2[i] = strdup((char*) vector_get(execArg, i));
			}

			myargs2[len] = NULL;
		}
		else
		{
			fprintf(stderr, "syntax error near unexpected token `|'");
		}

		vector_delete(execArg);





		//Initializing the pipeline

		if (pipe(pipefd) == -1) 
		{
		    perror("pipe");
		    exit(-1);
	  	}

	  	pid_ch1 = fork();
	  	pidNum[totalCom-1] = pid_ch1;
	 	if (pid_ch1 > 0)
	 	{
	    	//printf("Child1 pid = %d\n",pid_ch1);
	    	
	    	// Parent
	    	pid_ch2 = fork();
	    	if (pid_ch2 > 0){
	      		//printf("Child2 pid = %d\n",pid_ch2);
	      	
		   //    	if (signal(SIGINT, sig_int) == SIG_ERR)
					// printf("signal(SIGINT) error");
		      	
		   //    	if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
					// printf("signal(SIGTSTP) error");
		      	
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
					  //printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));
					  count++;
					  int i;
						for(i = 0; i < totalCom; i++)
						{
							if(pid_ch1 == pidNum[i])
							{
								finished[i] = 1;
								break;
							}
						}
					} 
					else if (WIFSIGNALED(status)) 
					{
					  //printf("child %d killed by signal %d\n", pid, WTERMSIG(status));
					  count++;
					} 
					else if(bgPogress == 1)
					{
						break;
					}
					else if (WIFSTOPPED(status)) 
					{
						printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
					// 	//printf("Sending CONT to %d\n", pid);
					// 	sleep(4); //sleep for 4 seconds before sending CONT
					// 	kill(pid,SIGCONT);
					// }
					// else if (WIFCONTINUED(status)) 
					// {
					// 	printf("Continuing %d\n",pid);
				    }
			    }
		      	return;
	    	}
			else 
			{
				//Child 2
				FILE *fpSTDOUT = NULL;
				char *filename = (char*) vector_get(filRedirection2, 1);
				if(filename != NULL) 
				{
					if ((fpSTDOUT =  fopen(filename, "w")) ==   NULL) 
					{
						fprintf(stderr,"can't open %s", filename);
					}
					else
					{
						dup2(5,STDOUT_FILENO);
					}
				}

				filename = (char*) vector_get(filRedirection2, 2);
				FILE *fpSTDERR = NULL;
				if(filename != NULL) 
				{
					if ((fpSTDERR =  fopen(filename, "w")) ==   NULL) 
					{
						fprintf(stderr,"can't open %s", filename);
					}
					else
					{
						dup2(6,STDERR_FILENO);
					}
				}

				vector_delete(filRedirection2);
				//sleep(1);
				setpgid(0,pid_ch1); //child2 joins the group whose group id is same as child1's pid
				close(pipefd[1]); // close the write end
				dup2(pipefd[0],STDIN_FILENO);
      			if(execvp(myargs2[0], myargs2) == -1)
				{
					fprintf(stderr, "%s: command not found\n", myargs2[0]);
				}
				else
				{
					vector_appendE(pidJobs, current_command);
					finished[totalCom] = 0;
					current = totalCom;
					totalCom++;
				}	


				if(fpSTDOUT != NULL)
				{
					fclose(fpSTDOUT);
				}
				if(fpSTDERR != NULL)
				{
					fclose(fpSTDERR);
				}
			}
		}
	  	else 
	  	{
			// Child 1
			setsid(); // child 1 creates a new session and a new group and becomes leader -
			//   group id is same as his pid: pid_ch1
			FILE *fpSTDIN = NULL;
			char *filename = (char*) vector_get(filRedirection1, 0);
			if(filename != NULL) 
			{
				if ((fpSTDIN =  fopen(filename, "r")) ==   NULL) 
				{
					fprintf(stderr,"can't open %s", filename);
				}
				else
				{
					dup2(5,STDIN_FILENO);
				}
			}

			filename = (char*) vector_get(filRedirection1, 2);
			FILE *fpSTDERR = NULL;
			if(filename != NULL) 
			{
				if ((fpSTDERR =  fopen(filename, "w")) ==   NULL) 
				{
					fprintf(stderr,"can't open %s", filename);
				}
				else
				{
					dup2(6,STDERR_FILENO);
				}
			}
			vector_delete(filRedirection1);
			close(pipefd[0]); // close the read end
			dup2(pipefd[1],STDOUT_FILENO);  
			
			if(execvp(myargs1[0], myargs1) == -1)
			{
				fprintf(stderr, "%s: command not found\n", myargs1[0]);
			}
			else
			{
				vector_appendE(pidJobs, current_command);
				finished[totalCom] = 0;
				current = totalCom;
				totalCom++;
			}	
			if(fpSTDIN != NULL)
			{
				fclose(fpSTDIN);
			}
			if(fpSTDERR != NULL)
			{
				fclose(fpSTDERR);
			}

		}
	}
	else
	{
		pid_ch1 = fork();
		pidNum[totalCom -1] = pid_ch1;
		if(pid_ch1 > 0) // parent
		{	
			if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
				printf("signal(SIGTSTP) error");	
			int count = 0;
	      	while (count < 1) 
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
				  //printf("child %d exited, status=%d\n", pid, WEXITSTATUS(status));
					int i;
					for(i = 0; i < totalCom; i++)
					{
						if(pid_ch1 == pidNum[i])
						{
							finished[i] = 1;
							break;
						}
					}
				  count++;
				} 
				else if (WIFSIGNALED(status)) 
				{
				  //printf("child %d killed by signal %d\n", pid, WTERMSIG(status));
				  count++;
				} 
				else if(bgPogress == 1)
				{
					current = totalCom-1;

					break;
				}
				else if (WIFSTOPPED(status)) 
				{
					//printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
				// 	//printf("Sending CONT to %d\n", pid);
				// 	sleep(4); //sleep for 4 seconds before sending CONT
				// 	kill(pid,SIGCONT);
				}
				// else if (WIFCONTINUED(status)) 
				// {
				// 	//printf("Continuing %d\n",pid);
				// }
		    }
	      	return;
		}

		else
		{
			
			Vector *filRedirection = (Vector*) vector_get(fileDArr, 0);

			FILE *fpSTDOUT = NULL;
			char *filename = (char*) vector_get(filRedirection, 1);
			if(filename != NULL) 
			{
				if ((fpSTDOUT =  fopen(filename, "w")) ==   NULL) 
				{
					fprintf(stderr,"can't open %s", filename);
				}
				else
				{
					dup2(3,STDOUT_FILENO);
				}
			}

			filename = (char*) vector_get(filRedirection, 2);
			FILE *fpSTDERR = NULL;
			if(filename != NULL) 
			{
				if ((fpSTDERR =  fopen(filename, "w")) ==   NULL) 
				{
					fprintf(stderr,"can't open %s", filename);
				}
				else
				{
					dup2(4,STDERR_FILENO);
				}
			}

			filename = (char*) vector_get(filRedirection, 0);
			FILE *fpSTDIN = NULL;
			if(filename != NULL) 
			{
				if ((fpSTDIN =  fopen(filename, "r")) ==   NULL) 
				{
					fprintf(stderr,"can't open %s", filename);
				}
				else
				{
					dup2(5,STDIN_FILENO);
				}
			}

			vector_delete(filRedirection);




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
				
				if(execvp(myargs[0], myargs) == -1)
				{
					fprintf(stderr, "%s: command not found\n", myargs[0]);
				}
			}
			if(fpSTDOUT != NULL)
			{
				fclose(fpSTDOUT);
			}
			if(fpSTDERR != NULL)
			{
				fclose(fpSTDERR);
			}
			if(fpSTDIN != NULL)
			{
				fclose(fpSTDIN);
			}
			vector_delete(execArg);
		}

	}	
}


void checkChild()
{
	pid = waitpid(-1, &status, WUNTRACED | WNOHANG);
	if(pid != 0)
	{
		int i;
		for(i = 0; i < totalCom; i++)
		{
			if(pid == pidNum[i])
			{
				finished[i] = 1;
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	
	pidJobs = vector_constructor(100);
	char input[2000];
	printf("# ");
	
	while(fgets(input, 2000, stdin) != NULL) 
	{
		current_command = strdup(input);
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
		
		fileDArr = vector_constructor(2);
		executeArr  = vector_constructor(0);
		setbuf(stdin, NULL);
		checkChild();
		int typeOfJob = processCommand(command);
		if(typeOfJob == 1)
		{
			for(int i =0; i < vector_len(pidJobs); i++) 
			{
				char* strpt = (char*)vector_get(pidJobs,i);
				if( strpt != NULL)
				{
					char* status;
					char* currentStat;
					if(finished[i] == 1)
						status = strdup("Done");
					else if(finished[i] == 2)
						status = strdup("Stopped");
					else
						status = strdup("Running");

					if(current == i)
					{
						currentStat = strdup("+");
					}
					else
						currentStat = strdup("-");
					printf("[%d]%s  %s                 %s\n", i+1,currentStat,status, strpt);

					free(strpt);
					free(status);
					free(currentStat);
				} 

			}
		}
		else if(typeOfJob == 2)
		{
			char *pidArgs = (char*) vector_get(command, 1);
			if(pidArgs == NULL)
			{
				if(finished[pidNum[current]] != 1)
				{
					kill(pidNum[current], SIGCONT);
				}
				else
					fprintf(stderr, "fg: %s : no such job", "current");
			}
			else
			{
				char *num =  strdup(pidArgs+1);
				int i = atoi (num);
				
				if(finished[pidNum[i]] != 1)
				{
					kill(pidNum[i], SIGCONT);
				}
				else
					fprintf(stderr, "fg: %%%d : no such job", i);
				free(num);
				free(pidArgs);

			}
		}
		else if(typeOfJob == 3)
		{
			char *pidArgs = (char*) vector_get(command, 1);
			if(pidArgs == NULL)
			{
				if(finished[pidNum[current]] != 1)
				{
					kill(pidNum[current], SIGCONT);
				}
				else
					fprintf(stderr, "bg: %s : no such job", "current");
			}
			else
			{
				char *num =  strdup(pidArgs+1);
				int i = atoi (num);
				
				if(finished[pidNum[i]] != 1)
				{
					kill(pidNum[i], SIGCONT);
				}
				else
					fprintf(stderr, "bg: %%%d : no such job", i);
				free(num);
				free(pidArgs);
			}
		}
		else
		{
			executeCommand();
			vector_delete(command);
			vector_delete(fileDArr);
			vector_delete(executeArr);
		}
		setbuf(stdin, NULL);
		printf("# ");
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
