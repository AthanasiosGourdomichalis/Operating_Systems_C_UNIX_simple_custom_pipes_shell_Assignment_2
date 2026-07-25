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

	}

	free(input);
	return 0;
}