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

	while(1){
		printf("$ ");

		line = getline(&input, &length, stdin);
		if(line == -1){ //ERROR
			break;
		}
		
		char* cmd = strtok(input," \t\n"); //no spaces
		if (cmd == 0) //null string
		{
			continue;
		}
		if (strcmp(cmd, "exit") == 0){
			break;
		}

		pid_t pid;
		pid = fork();

		if (pid<0) {
			perror("Fork error"); 
			continue;
		}
		if (pid == 0) //child process
		{ 
			execlp(cmd, cmd, NULL);
			perror("Command not found"); 
			exit(1);
		}
		else //parent process waits for the command to finish, before the next while-loop
		{
			waitpid(pid, NULL, 0);//NULL pointer to int* status, because the return status is not needed
								  //wait for child procces with process_id = pid
								  //option = 0
		}
	}

	free(input);
	return 0;
}