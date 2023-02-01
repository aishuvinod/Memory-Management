#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_SIZE 255
// defines the maximum possible characters of a single line as a global constant
#include "tokens.h"
#include "vect.h"

// executes a command by using a vector that we get the commands from
int exec_command(vect_t* vector) {
		char** commands[vect_size(vector) + 1];
                //an array that takes in the vector
                int counter = 0;
                int init_size = vect_size(vector);
                while (counter < init_size) {
                        commands[counter] = vect_get(vector, counter);
                        counter = counter + 1;
                }
                commands[counter] = NULL;
                int ex = execvp(commands[0], commands);
                exit(1);
	
}

// executes a command if its a semicolon or null input
int exec_command_semi_or_null(vect_t* vector) {
       // pid returns the child process id
	int pid = fork();
	int child_status = 1;
	if (pid == 0) { //child
		exec_command(vector);
	}
	wait(&child_status); //holds the child's exit status
	if (child_status != 0) {  //no process is found
		printf("%s: command not found\n", vect_get(vector, 0));
    	}
}

//function which executes a command which is followed by either a "<" or ">"  and a filename. 
// if the command is "<" and a filename then the command will be run with the contents of he file replacing the initial input
// if the command is ">" and a filename then the command will be run and the output will be stored in the given filename
int exec_command_redirect(vect_t* vector, char* path) {
        int pid = fork();
        int child_status = 1;
        if (pid == 0) {
		close(1);
                //create the file and truncate it if it exists
                int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644); //open file descriptor
		if (fd == -1) { 
			exit(1);
		}
		exec_command(vector);
        }
        wait(&child_status);
        if (child_status != 0) {
                printf("%s: command not found\n", vect_get(vector, 0));
        }
}

// function which executes any given command while keep track of and storing each command
int exec(char* command, vect_t* previous_commands) {
	char ** tokens = get_tokens(command);
        vect_t *vect = vect_new();
	int i = 0;
        //while there is an argument being entered
	while (tokens[i] != NULL) {
		if (strcmp("exit", tokens[i]) == 0) {
        //check if the entered argument is exit
			vect_delete(previous_commands);
                        //delete all the temporarily stored previous commands used for the previous command
			printf("Bye bye.");
			exit(1);
                        //exit shell
		}
                // checks if user enters source as a command
                // if there is a filename after the source comamnd then run that file line by line as an argument
		else if (strcmp("source", tokens[i]) == 0) {
			if (tokens[i+1] == NULL) { // second argument which should be the file name
                                printf("Error: need a file descriptor\n");
                        } else { //if the file is not null
				char buffer[256]; //readable file size  
				int length = read(open(tokens[i+1], O_RDONLY), buffer, 255);
                                // open and read the file
 				buffer[length] = '\0';
                                // initializing the string to start reading
                                exec(buffer, previous_commands); //execute the read lines
                                i = i + 1; 
                        }
		}
                // if the user enters prev as a command then read the 
		else if (strcmp("prev", tokens[i]) == 0 && vect_size(previous_commands) > 0) {
                //if the user enters prev and if there was any previous commands entered 
			vect_t* new_previous_commands = vect_new(); //add new argument to the stored previous commands collection
			int counter = 0;
			while (counter < vect_size(previous_commands) - 1) {
				vect_add(new_previous_commands, vect_get(previous_commands, counter));
				counter += 1; 
			}
                        printf("%s\n", vect_get(previous_commands, vect_size(previous_commands) - 1));
			exec(vect_get(previous_commands, vect_size(previous_commands) - 1), new_previous_commands);
		}
		else if (strcmp("cd", tokens[i]) == 0) {
			chdir(tokens[i+1]);//change directory to the given directory argument
			i = i + 1;
		}
		 else if(strcmp("help", tokens[i]) == 0){
                 //prints out the available built in commands
                       printf("These are the built in commands:\n");

                       printf("cd : change your current directory.\n");

                       printf("source: takes in a file as a filename and processes each line.\n");

                       printf("prev: prints out the previous command line argument and executes it.\n");

                }
                // executes commands on either size of the ; separately
		else if (strcmp(";", tokens[i]) == 0 && vect_size(vect) > 0) {
			exec_command_semi_or_null(vect);
			vect_delete(vect);
			vect = vect_new();
		}
                //input will be outputted in the given file
		else if (strcmp(">", tokens[i]) == 0 && vect_size(vect) > 0) {
			if (tokens[i+1] == NULL) { //check if the file descriptor argument is null
				printf("Error: need a file descriptor\n");
			} else {
				exec_command_redirect(vect, tokens[i+1]); //execute redirect on the file
				i = i + 1;
			}
			vect_delete(vect);
			vect = vect_new();
		}
                //input will be replaced by whatever is in the file after the "<"
		else if (strcmp("<", tokens[i]) != 0 && strcmp(";", tokens[i]) != 0 && strcmp(">", tokens[i]) != 0) {
			vect_add(vect, tokens[i]);
		}
		i = i + 1;
        }
	if (vect_size(vect) > 0) { 
                exec_command_semi_or_null(vect);
        }
	vect_delete(vect);
	free_tokens(tokens); //free memory 
}

// main function
int main(int argc, char **argv) {
  
  vect_t* prev_commands = vect_new();
  char command[MAX_SIZE];//assigning global constant size
    printf("Welcome to mini-shell.\n"); 
    while (1) { //run loop forever 
        printf("shell $");
	if(fgets(command, MAX_SIZE, stdin) == NULL){
        // if the user enters control D, the program detects it as NULL
                        printf("Bye bye.");
			vect_delete(prev_commands); 
                        exit(1);
                        //exit shell and delete stored previous commands
        }
	exec(command, prev_commands);
	if (strcmp("prev", command) != 0 && strcmp("prev;", command) != 0) {
		vect_add(prev_commands, command);
	}
    }
    return 0;
}
