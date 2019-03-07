#include<stdio.h>
#include<stdlib.h>
//#include<stdbool.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#define N 666

void parsing(char *a, char **b);      		    //declare a prototype function to parse input args as commands 

int main() {
	int  i = 0, pid = 0, status;
	char input[N];
	int pfd[2];
	char *arg[N];
	char *args[N];
	while (1) {                                //while(true) shell loops infinitely until user commands exit 
		printf("shell$ ");              	//display the shell $ indicating the shell is ready for command
		fgets(input, N, stdin);	 	 //reads a line up until size N from console and stores it in array input
		parsing(input, arg);           	//calls for the parse prototype function in order to extract valid commands 
		while (arg[i] != NULL) {                            //loop until the input = null


				/*Output Redirection*/
			if (strcmp(arg[i], ">") == 0) {          	//if string compare is a match..
				pid = fork(); 		 		// create a child process 
				if (pid == -1) {                	//if fork() returns a -1, to the parent process, no child is created 
					perror("Failed to fork");   		//print out an error message (even though global variable errno can do it as well). 
					exit(EXIT_FAILURE);       	//exit
				}
				else if (pid == 0) {                 	//if fork() returns 0, child process is created  &  process ID of child process is returned to parent 
					int output = creat(arg[i + 1], 0666);        //creates a file (if it doesnt yet exist)  with rw-rw-rw permission (mode) 
					dup2(output, 1);                             // duplicates the file, returns 1 
					close(output);                               //closes the original 
					arg[i] = NULL;                            //sets i'th element to NULL
					arg[i + 1] = NULL;
					execvp(arg[0], arg);                      //executes command at arg[0], in this case echo, terminated by NULL at arg[i+1]
				}
				else if (pid > 0) {                  		  //otherwise 
					waitpid(pid, &status, 0);      		//wait for child process to terminate 
				}
			}

				/*Input Redirection*/
			if (strcmp(arg[i], "<") == 0) {                    	 //same as commented above above
				pid = fork();
				if (pid == -1) {
					perror("Failed to fork");
					exit(EXIT_FAILURE);
				} 
				else if (pid == 0) {                                
					int fd = open(arg[i + 1], O_RDONLY);     //opens a file with specified pathname with read-only access mode
					dup2(fd, STDIN_FILENO);                  //duplicates this file, returns the value of stdin 
					close(fd);                               //closes original 
					arg[i] = NULL; 
					arg[i + 1] = NULL;
					execvp(arg[0], arg);                    //executes command at arg[0], in this case cat, also terminated by NULL at arg[i+1]
				}
				else if (pid > 0) {                              //otherwise
					waitpid(pid, &status, 0);              //wait for child process to terminate
				}
			}
       				/*Pipe*/
			if (strcmp(arg[i], "|") == 0) {                
				int a = 0;                              
				char *argB[2];                                //create char arrays to contain the commands connected by a pipe
				char *argA[2];  
				while (a < 2) {                                //while the command char limit is less than 2
					argA[a] = arg[a];                         //first command is at arg[0]
					argB[a] = arg[a + 3];                        //secondc command is at arg[3]
					a++;                                            //increment a to iterate through the entire line
				}
				pipe(pfd);                                             //create a pipe
				pid = fork();                                            //create a child process for the pipe
				if (pid == 0) {
					dup2(pfd[0], 0);                                  //duplicate the process
					close(pfd[1]);				 //close the original
					execvp(argB[0], argB);			//execute 
				}
				else {
					dup2(pfd[1], 1);
					close(pfd[0]);
					execvp(argA[0], argA);
				}
			}
				/*Background*/
			if (strcmp(arg[i], "&") == 0) {                       
				pid = fork();
     				if(pid == -1){
					perror("Failed to Fork"); 
					exit(EXIT_FAILURE);
				}			 	       	//create a child process
				else if (pid == 0) {
					//arg[i] = NULL;
					//arg[i+1] = NULL;
					//if successful
					execvp(arg[0], arg);		 //execute 
				}
				else if (pid > 0){
					waitpid(pid, &status, 0);
				}
			}
				/*Extra Credit*/
			/* Note that these system calls are often used with directories, however I did not implement mkdir in this project.*/
			if (strcmp(arg[i], "cd") == 0) {           
				char *directory = arg[i + 1];                   //create a pointer corresponding to the memory address of the desired directory 
				int p = chdir(directory);    	       		 //changes working directory to the spefified arg[i] address 
				execvp(arg[0], arg); 
			}
			if(strcmp(arg[i], "ls") == 0){
				//char *directory = arg[i + 1];
				int d = opendir(".");                               //opens the current directory 
				printf("Files in current directory:\n %s\n", readdir(d));   //prints all the files in the current directory, 0 if none. 
			}
			if (strcmp(arg[i], "pwd") == 0) {
				char path[6*N];                                // create a char array to store the path 
				getcwd(path, sizeof(path)); 		       //returns the absolute pathname of the current working directory of the calling process
				printf("Current directory path: %s\n", path);  //prints the pathname 
			}

				/*Exit*/
			if (strcmp(arg[i], "exit") == 0) {
				printf("exiting...\n");                        
				exit(0);				        //exit the shell 
			}
			i++;
		}
		i = 0;
	}
}

void parsing(char *in, char **arg) {                                                //parsing prototype function to extract commands from user input
	while (*in != '\0') { 					   //while input is not null
		while (*in == ' ' || *in == '\n' || *in == '\t')               //while input contains whitespace chars
			*in++ = '\0';                                         //iterate and replace all whitespace chars with null
		*arg++ = in;					   
		while (*in != '\0' && *in != ' ' && *in != '\t' && *in != '\n')  //as long as there are no more whitespace chars remaining 
			in++;                                                //traverse through the input 
	}
	*arg = '\0';                                
}