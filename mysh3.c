#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

	//arguments for getline(3): 
	char* input = 0; //NULL
	size_t length = 0; 
	ssize_t line;
	char cwd[1024]; //used in getcwd(3)

	while(1){
		printf("$ ");

		line = getline(&input, &length, stdin);
		if(line == -1){ //ERROR
			break;
		}
		
		
		int counter = 0;
		int indx = -1;
		while(input[counter] != '\0') {
			if (input[counter] == '|'){ //search for pipe appearance
				indx = counter;
				break;
			}
			counter++;
		}

		if (indx == -1){ // NO PIPES (same as mysh2.c) 

			char* tokens = strtok(input," \t\n"); //no spaces
			if (tokens == NULL){ continue; } //NULL string, no commands

			char** arguments = malloc((line+1)*sizeof(char*)); //we will parse it into execvp
									//line+1 = input size (from getline()) +1 position for NULL
			int i = 0;
			while(tokens != 0 ){ //NOT NULL 
				arguments[i] = tokens; //every command parameter (e.g. "-l", "/tmp", etc)
				i++;
				tokens = strtok(NULL, " \t\n"); //from where the tokenizing stopped, based on the char. '\0'
			}

			arguments[i] = NULL; //based on the last value of the i counter, we assign the NULL value in the array's last position, for execvp()
			
			if (strcmp(arguments[0], "exit") == 0){ //key-word
				free(arguments);
				break;
			}

			if (arguments[0] != NULL && strcmp(arguments[0], "cd") == 0 ){ //NOT NULL
				if(arguments[1] != NULL){
					if(chdir(arguments[1]) != 0){ //FAIL
						perror("cd error");
					}else{
						if(getcwd(cwd, sizeof(cwd)) != NULL ){
							printf("[Current working directory: %s]", cwd); //pwd
						}
					}
				} else {
					perror("File or directory not found"); //cd without a path!
				}
				free(arguments);
				continue; //next while loop - in the changed directory (before fork())
			}
			pid_t pid;
			pid = fork();

			if (pid<0) {
				perror("Fork error");
				free(arguments); 
				continue;
			}
			if (pid == 0){ //child process
				execvp(arguments[0], arguments); //with string tokenize, in the 1st position we assign the executable name and in the following ones we assign the parameters
				perror("Command not found");
				free(arguments); 
				exit(1);
			}
			else //parent process waits for the command to finish, before the next while-loop
			{
				waitpid(pid, NULL ,0);//NULL pointer to int* status, because the return status is not needed
								  //wait for child procces with process_id = pid
								  //option = 0
			}
			free(arguments);
//------------------------------------------------------------------------			
		} else { //PIPES

			input[indx] = '\0'; //delimiter between the 2 piped commands
			char* cmd_1 = input; //stops at '\0'
			char* cmd_2 = &input[indx+1]; //next memory address after '\0'

			char** args_1 = malloc((line+1)*sizeof(char*)); //we will parse it into execvp
			char** args_2 = malloc((line+1)*sizeof(char*)); //we will parse it into execvp

			//-----------------------------------------------------------
			char* tokens_1 = strtok(cmd_1, " \t\n"); //no spaces

			if(tokens_1 == NULL){
				perror("Command not found");
				free(args_1);
				free(args_2);
				continue;
			}

			int i = 0;
			while(tokens_1 != 0 ){ //NOT NULL 
				args_1[i] = tokens_1; //every command parameter (e.g. "-l", "/tmp", etc)
				i++;
				tokens_1 = strtok(NULL, " \t\n"); //from where the tokenizing stopped, based on the char. '\0'
			}
			args_1[i] = NULL; //based on the last value of the i counter, we assign the NULL value in the array's last position, for execvp()
			//-----------------------------------------------------------
			char* tokens_2 = strtok(cmd_2, " \t\n"); //no spaces

			if(tokens_2 == NULL){
				perror("Command not found");
				free(args_1);
				free(args_2);
				continue;
			}

			int j = 0;
			while(tokens_2 != 0 ){ //NOT NULL 
				args_2[j] = tokens_2; //every command parameter (e.g. "-l", "/tmp", etc)
				j++;
				tokens_2 = strtok(NULL, " \t\n"); //from where the tokenizing stopped, based on the char. '\0'
			}
			args_2[j] = NULL; //based on the last value of the j counter, we assign the NULL value in the array's last position, for execvp()
			//-----------------------------------------------------------

			pid_t pid1, pid2;
			int fd[2];

			if (pipe(fd)<0) {
				perror("Pipe error");
				free(args_1); 
				free(args_2);
				continue;
			}

			pid1 = fork();
			if (pid1<0) {
				perror("Fork error");
				free(args_1); 
				free(args_2);
				continue;
			}
			if(pid1==0){ //child 1 (before pipe)
				dup2(fd[1], 1); //change out direction
				close(fd[0]); //close reader
				close(fd[1]); //close writer
				execvp(args_1[0], args_1); //with string tokenize, in the 1st position we assign the executable name and in the following ones we assign the parameters
				perror("Error in execvp() 1"); 
				exit(1);
			}

			pid2 = fork();
			if (pid2<0) {
				perror("Fork error");
				free(args_1); 
				free(args_2);
				continue;
			}
			if(pid2==0){ //child 2 (after pipe)
				dup2(fd[0], 0); //change in direction
				close(fd[0]); //close reader
				close(fd[1]); //close writer
				execvp(args_2[0], args_2); //with string tokenize, in the 1st position we assign the executable name and in the following ones we assign the parameters
				perror("Error in execvp() 2"); 
				exit(1);
			}

			close(fd[0]);
			close(fd[1]);
			waitpid(pid1, NULL ,0);//NULL pointer to int* status, because the return status is not needed
								  //wait for child procces with process_id = pid1
								  //option = 0
			waitpid(pid2, NULL ,0);//wait for child procces with process_id = pid2

			free(args_1);
			free(args_2);
		}

	}

	free(input);
	return 0;
}