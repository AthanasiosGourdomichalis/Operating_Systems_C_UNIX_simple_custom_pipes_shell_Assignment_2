# Operating_Systems_C_UNIX_simple_custom_pipes_shell_Assignment_2
The general structure across all three mysh implementations consists of the main() function 
containing an infinite while(1) loop that continuously displays the prompt "$ " for the ongoing 
execution of the shell. This loop terminates upon receiving the "exit" command or 
encountering an unrecoverable error (via break), such as an execution failure in exec(), etc. 
Furthermore, in the case of invalid input, the shells do not hang or crash, nor do they print 
anything to stdout; instead, error messages are routed to stderr using perror() (e.g., 
"Command not found"). I have implemented all parts of the assignment, and regarding the 
automated test scripts for submission, everything passes correctly. Finally, comprehensive 
inline comments have been provided throughout most of the codebase for further 
clarification. 
Issues Encountered During Implementation & Solutions. For mysh[123].c:

1. mysh1.c: I read the program name from standard input using getline(3), passing a  
char *input buffer pointer, a size_t length for the buffer capacity, and storing the return 
value in an ssize_t line variable (which includes the trailing newline \n).

o Issue & Fix: Initially, I faced failures on automated Tests 15–18 because I had not 
accounted for input with leading whitespaces (e.g., "____ls" instead of "ls"). I 
resolved this by using strtok(3)-as suggested in the project specifications
passing input and " \t\n" as the delimiter set (spaces, tabs, and newlines). 
After testing various edge cases before and after the fix, the issue was 
resolved and the tests passed. 

o Empty Commands & fork() Failures: For empty lines or rare fork() errors, the 
execution simply proceeds to the next iteration using continue. 

o Process Execution: Inside the child process, I chose execlp(3) because we only 
pass a single parameter list (the command name followed by NULL), and it 
automatically searches the directories listed in the $PATH environment variable.

o Synchronization: The parent process calls waitpid(3) to wait for the specific child 
process to finish before displaying the prompt again for the next command. 

o Memory Management: Finally, I call free(input) at the end. Researching C 
library behavior revealed that getline(3) internally allocates dynamic memory 
via malloc when passed a NULL buffer or zero size, so freeing it prevents 
memory leaks. 

2. mysh2.c: Building upon mysh1, mysh2 adds support for command-line arguments.
  
o Argument Parsing: Input tokens are initially extracted using strtok(3) with " \t\n" 
as delimiters. Inside a while loop, subsequent tokens are parsed and sequentially 
stored into a dynamically allocated arguments array.
 
o Process Execution: Because command-line arguments are stored as a 
dynamically populated array of pointers, I replaced execlp(3) with execvp(3). 

o Built-in cd Command: I added explicit logic to handle the cd built-in command: 

o Error checking for non-existent files or directories when 
chdir(arguments[1]) != 0. 

o Handling cases where the user supplies cd without an argument 
(arguments[1] == NULL).

o Optionally inspecting the current working directory after a successful chdir 
call using getcwd(3) with a char cwd[1024] buffer. 

o Memory Management: The process creation logic using fork() and checking the 
return pid_t remains identical to mysh1. However, whenever the control flow 
changes (e.g., exiting, handling cd, or encountering errors during fork/exec), I 
explicitly call free() on the arguments array to avoid memory leaks before entering 
the next loop iteration. 

o Issue & Fix: The primary issue I encountered was initially using a fixed-size 
static array for the exec argument vector (e.g., 64 or 80 elements). This 
resulted in test failures until I received the automated test hints: 

"Test 10/37: fail... (HINT: Are you using dynamic allocation?)" and 
"Test 21/37: fail... (HINT: Are you using dynamic allocation?)" 

Switching to dynamic memory allocation resolved these issues. Furthermore, studying 
the manual pages for chdir(2) and getcwd(3) proved invaluable for implementing 
directory navigation correctly.

3. mysh.3: Building upon mysh1 and mysh2, mysh3 adds support for pipes.
    
o Parsing & Splitting: After reading user input via getline(3), I iterate through the char 
array (C-string) searching for the pipe delimiter character '|'. If no pipe is found, 
execution falls back seamlessly to the exact same logic used in mysh2.c. If a pipe 
is found, I replace the '|' character with a null terminator ('\0'). This allows me to 
assign char *cmd_1 to the first command preceding the pipe (as reading stops at 
'\0'), while char *cmd_2 points to the second command immediately following the 
'|', leveraging the contiguous memory layout of the original input buffer. I then apply 
the exact parsing logic from mysh2 using strtok(3) on both individual commands, 
including identical error checks and dynamic memory allocations/deallocations. 

o Process Creation & Inter-Process Communication (IPC): To execute the piped 
commands, I declare two process IDs (pid_t pid1, pid2) for the two child 
processes and an integer array int fd[2] to store the read and write file descriptors 
for the pipe. Appropriate error handling is included for pipe(2) and both fork(2) 
calls, along with proper dynamic memory cleanups. 

o Redirection using dup2(2): The primary challenge (though no automated test 
cases failed) was understanding how to connect the two processes via dup2(2) so 
that the stdout of the first command flows into the stdin of the second command: 

o In the first child process (executing cmd_1), I redirect standard output 
(stdout) to fd[1] (the write end of the pipe) using dup2(2). 

o In the second child process (executing cmd_2), I redirect standard input 
(stdin) to fd[0] (the read end of the pipe) using dup2(2). 

o Closing File Descriptors: Immediately after invoking dup2(2), unused file 
descriptors are closed within each child. In the first child, fd[0] is closed because it 
does not read from the pipe, and fd[1] is closed as it is redundant after duplication. 
In the second child, fd[1] is closed because it does not write to the pipe, and fd[0] 
is closed after dup2(2). 

o Parent Process Cleanup: Most importantly, both pipe file descriptors (fd[0] and 
fd[1]) must also be closed in the parent process outside the child conditional 
blocks. If the parent keeps fd[1] open, the second child process will hang 
indefinitely waiting for more input (EOF will never be sent), assuming the parent 
might still write to the pipe. This causes the shell to freeze/hang and fail to print the 
next prompt ("$ "). 

In particular, the manual pages for dup(2) and dup2(2) proved especially helpful.
