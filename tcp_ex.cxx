#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <iostream>
#include <ctime>


using namespace std;



/*
*
* Purpose: Create a simple file server using c++ system calls
* file server will use TCP infrastructure and will be reachable
* via local host ports. The server expects a port number and a
* path for the folder which will be the file server as a 
* command line argument when it is started from shell! The server
* checks for GET command along with the path provided by the client.
* If invalid path is provided the server will disregard the comand.
* Exceptions are given if extra / is at the end of the path.
*
*
* INFO command has been implemented
* 
*/


//examine port argument if it is all digits
bool test_port_num(char*);

//examine the pathname and parse it
void path_name(int & , char *, char *);
//void path_name(int sok, char * message, char * org_path)

int main(int argc, char *argv[])
{
	//test for the correct number of command line arguments
	if(argc <= 1)
	{
		printf("Required arguments are not present! \n");
		printf("Please enter port followed by path\n");
		
		exit(EXIT_FAILURE);
	}
	
	//test if port is all digits
	bool port_test = test_port_num(argv[1]);
	
	//if port is not all digits show error and exit
	if (port_test == false)
	{
		printf("Invalid Port Number!");
		exit(EXIT_FAILURE);
	}
	
	//server pathname
	struct stat server_stat;
	
	//check the folder path for the server
	int server_res = stat(argv[2],&server_stat);
	
	
	//if stat failed produce a readable error
	if(server_res < 0)
	{
		perror("Stat");
		exit(EXIT_FAILURE);
	}
	
	//looking for directory on server side
	//if not a directory exit with error
	if(!S_ISDIR(server_stat.st_mode))
	{
		cout << "A directory was not selected" << endl;
		exit(EXIT_FAILURE);
	}
		

	//network infrastructure is created in main
	//sub functions will provide checking and return
	//the tokens as they should be processed by the server

	int sock, new_socket, attend, serverlen;
	
	ssize_t received;	
	char buffer[1024];

	struct sockaddr_in address;


	//create a socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	serverlen = sizeof(address);

	//check if the socket was successfully created
	if (sock < 0)
	{
		//if -1 returned output error.
		perror("Cannot connect!");
		//exit with fail
		exit(EXIT_FAILURE);
	}

	//Construct the server sockaddr infrastructure
	memset(&address, 0, sizeof(address)); 		//clear the struct
	address.sin_family = AF_INET;			//internet ip
	address.sin_addr.s_addr = INADDR_ANY;		//any ip address
	address.sin_port = htons(atoi(argv[1]));	//server port

	//second structure to use in while loop
    struct sockaddr_in address2;
    
    //second structure to use with accept
    socklen_t addrlen2;
    
    //second adderlen to use with accept
    addrlen2 = sizeof(struct sockaddr_in);

	//build the socket
	int res = bind(sock, (struct sockaddr *) &address, serverlen);
	
	//test if server was successfully built
	if (res < 0)
	{
		//if bind returns a -1 error
		perror("Failed to build server");
		exit(EXIT_FAILURE);
	}
	
	//listen for any connection passively
	attend = listen(sock, 64);
	
	while(true)
	{
		
		//if listen returns -1 exit with error
		if(attend < 0)
		{
			//display listening error
			perror("Listen Failed");
			
			//exit with fail
			exit(EXIT_FAILURE);
		}

		//accept connection and create a new socket
		 new_socket = accept(sock, (struct sockaddr *) &address2, &addrlen2);
	
		//if acccept failed new_socket will be -1
		if (new_socket < 0)
		{
			//display error
			perror("Failed to accept");
			//exit with fail
			exit(EXIT_FAILURE);
		}

		//after accept has occured fork
		//the parent process will continue listening
		//the child will do what is required by client

		if(fork())
		{
			//parent process

			close(new_socket);
		}
		else
		{
			//child process
				
			//read from socket into buffer of 1024 bytes
			received = read(new_socket,buffer, 1024);

			//if read fails it returns -1
			if(received < 0)
			{
				//display error
				perror("Read");

				//exit with fail
				exit(EXIT_FAILURE);
			}
			
			//make sure that buffer is null terminated
			buffer[received] = '\0';
			
			//call to function path name passing the writing socket
			//to it along with received information from client
			//and the folder of the webserver in argv[2]
			path_name(new_socket, buffer, argv[2]);	
			
			close(new_socket);
			exit(EXIT_SUCCESS);
		}
	}
	
	//close the connection socket
	close(sock);
	exit(EXIT_SUCCESS);
	
	//return 0 if no abend occured
	return 0;

}

/*
 * Test port number function is responsible for testing the port 
 * entered by the user at the run of the webserver. The only 
 * criteria that the function tests against is that the port 
 * consists only of digits.
 */
bool test_port_num(char * port)
{
	//size is the length of the string
    size_t len = strlen(port);
    
    //flag will be flipped to true or false
	int flag;
	
	//step through each character and evaluate
	for (unsigned int j = 0; j < len; ++j)
	{
		
		flag = isdigit(port[j]);	
		
		//if a non digit character is found immediatly return false
		if (flag ==0)
		{
			// return false as soon as non digit was foud
			return false;

		}
		
	}
	
	//else return true out of the function
		return true;
}

/*
 * Function pathname performs checing on the path that the client has 
 * sent and opens the directories using opendir system call. In addition
 * this function does the necessary check for index.html file and writes
 * it to the socket passed down by reference. If any system call fails
 * perror is called and the program exits since it is necessary that all
 * data gets to the client. Errors are displayed only on the servers side
 * 
 * Function receives 3 aguments - socket obtained from accept, the client
 * message and the server path through pointers to character arrays.
 * 
 * Function does not return but it can output to the console on the server
 * side.
 * 
 */

void path_name(int &sok, char * message, char * org_path)
{
		
	
	//error message sent to the client if wrong command is given
	char wcommand[] = "Invalid command! Commands allowed GET or INFO\n";
		
	//obtain the get commannd from the message
	char get_command[4];
	
	//obtain info command from the message
	char info_command[5];
	
	//holds only the path -- no command
	char tot_path[1024];
	
	//read into buffer to send only 1024 at a time
	char read_buf[1024];
	
	//pointer to first slash in path
	char * first_slash;
	
	//pointers to file names from read dir
	//max 100 files/folders per directory
	char * file_names[100];
	
	//has the user specified a directory with / at the end
	bool flag_is_dir = false;
	
	//is info file found in the folder
	bool info_found = false;
	
	//count of files in directories
	int file_count = 0;
		
	//copy first 3 characters into get command
	strncpy(get_command,message, 3);
	
	//copy first 4 characcters into info command
	strncpy(info_command,message, 5);
	
	//bytes written to the socket
	//will be cast to unsigned int to satisfy comparison warnnings
	int written;
	
	//if GET command is not located at the beginnig
	//exit out with an error
	if(((strncmp(get_command, "GET", 3)) != 0) && 
	    (strncmp(info_command, "INFO", 4)) != 0)
	{
		//write error to socket
		write(sok, wcommand, strlen(wcommand));
		exit(EXIT_FAILURE);
	}
		
	if(strncmp(info_command,"INFO",4) == 0)
	{
		//call raw time off of ctime
		time_t result = time(NULL);
		
		//convert into readable format using ctime
		char * time_buf =ctime(&result);
		
		//write header to the socket
		written = write(sok,"Current Time and Date\n",22);
		
		//if write fails call perror
		if(written < 0)
		{
			perror("Write");
			exit(EXIT_FAILURE);
		}
		
		//error check - header
		if(written != 22)
		{
			cout << "Writing Date message failed!" << endl;
		}
				
		//send string over socket
		written = write(sok,time_buf,strlen(time_buf));
		
		//if write fails call perror
		if(written < 0)
		{
			perror("Write");
			exit(EXIT_FAILURE);
		}
				
		//time and date error check
		if ( (unsigned int) written != strlen(time_buf))
		{
			cout << "Writing time to client failed!" << endl;
		}
		
		//return out of the function
		return;
	}
	
	if( message[4]  != '/')
	{
		//check for leading slash on path
		
		//write the error to the socket
		written = write(sok, "Path did not sstart with /\n", 27);
		
		//test if write failed report error
		if(written < 0)
		{
			perror("Write");
		}
		
		cout <<"Path did not start with /";
		exit(EXIT_FAILURE);
	}
	if(message[5] =='.' && message[6] == '.')
	{
		written = write(sok, 
		"Going back to the previous directory is not allowed!\n",53);
		
		//check if system call write failed and report error
		if(written < 0)
		{
			perror("Write");
		}
		
		cout << "Going back to the previous directory is not allowed!\n";
		exit(EXIT_FAILURE);
	}

	//flip the is a directory flag if the client gave / at the end
	//of the input to signal a directory
	if (message[((strlen(message))-1)] == '/')
		flag_is_dir = true;
	
	//find the position of the first slash
	first_slash = strchr(message,(int) '/');
	
	//length of string after first slash
	int first_s_len = strlen(first_slash);
	
	//if directory - remove slash char
	if (flag_is_dir)
	{
		first_s_len = first_s_len - 1;
	}
	
	//set total path array to zero	
	memset (tot_path, '\0',	strlen(tot_path));

	//copy arv[2] into total path
	memcpy(tot_path, org_path, strlen(org_path));
	
	//copy rest of the path in total path
	memcpy(tot_path+ strlen(org_path), first_slash, (first_s_len-2));
	
	//null terminate the total path array
	tot_path[strlen(tot_path)] = '\0';
	
	//create the structure with the file info	
	struct stat filestat;
	
	//call stat system call
	int stat_res = 	stat(tot_path,&filestat);
	
	//if stat fails --- no file or directory exists
	if (stat_res < 0)
	{
		written = write(sok,"No file or directory exists!\n",29);
		
		//if write fails perror
		if(written < 0)
		{
			perror("Write");
		}
		
		//report error on stat and exit
		perror("Stat");
		exit(EXIT_FAILURE);
	}
	
	if(S_ISDIR(filestat.st_mode))
	{
		//directory was given
		
		//construct the open dir stream
		DIR * d;
		
		//create a dirent struct to hold open dir pointer
	  	struct dirent *dir;
	  	
	  	//call open dir
	 	d = opendir(tot_path);
	 	
	 	//check if an error occured with openning the directory
	 	if(d == 0)
	 	{
			written = write(sok, "Directory failed to open!\n",26);
			
			//check if write failed if it did report error
			if(written < 0)
			{
				perror("Write");
			}
			
			//report error in opening the directory and exit
			perror("Open Directory");
			exit(EXIT_FAILURE);
		}
			
	 	
	 	//if d is true
	  	if (d)
	  	{
				//decend into the directory and check all contents
	    		while ((dir = readdir(d)) != NULL)
	    		{
	      			
					//if index.html is available
					//send the file through the socket
					if(strcmp(dir->d_name, "index.html") == 0)
					{
						info_found = true;
					}
					else
					{
						//If no info file was located send the contents
						//of the directory through the socket
						//excluding the hidden files and prev dir
						if ( dir->d_name[0] != '.' 
						&& dir->d_name[1] != '.') 
						{
							
							//save the file names into the pointer array
							file_names[file_count] = dir->d_name;
							
							//increment the count
							file_count++;
							
						}
					}		

	    		}
				//close directory at total path
	    		closedir(d);
	    		
	    		if(info_found)
	    		{
					//add index.html to the path
					memcpy(tot_path+ strlen(tot_path), "/index.html" , 
					11);
					
					//open index.html
					int open_fd = open(tot_path,O_RDONLY);
					
					//if open does not return a file decriptor
					//write an error to the socket and
					//print one on the server side
					if (open_fd < 0)
					{
						written = write(sok,"Index.html failed to open!\n",27);
						
						//if write fails report perror
						if(written < 0)
						{
							perror("Write");
						}
						
						//server side error
						perror("Open");
						exit(EXIT_FAILURE);
				
					}
					//read file into 1024 buffer and write it to the sock
					//do this until zero bytes are read in or 
					//writing to the socket fails.
					while(true)
					{
						
						//read the file with the opened filed decriptor
						ssize_t b_read = read(open_fd, read_buf, 
						sizeof(read_buf));
						
						//get out of while condtion
						if(b_read == 0)
						{
							break;
						}
						
						//if read fails -1 is returned display error
						if (b_read < 0)
						{
							written = write(sok, "File cannot be read!\n",21);
							
							//if write fails show error
							if( written < 0)
							{
								perror("Write");
							}
							
							perror("Read");
							exit(EXIT_FAILURE);
						}
										
						//write to the socket
						written = write (sok, read_buf, b_read);
						
						//check if write failed if it did report
						//error and exit the program
						if( written < 0)
						{
							perror("Write");
							exit(EXIT_FAILURE);
						}
						
						//if the bytes that were read are not the same
						//as the ones written exit with fail and message
						if(written < b_read)
						{
							cout << "Failed to send index.html file contents\n";
						}						

					}
					
					return;
				}
				else
				{
                    write(sok, "\n\n", 2); // http compatibility
					written = write(sok, "Files/Folders in Directory\n\n",
					28);
					
					
					//test if write system call failed
					if(written < 0)
					{
						perror("Write");
						exit(EXIT_FAILURE);
					}
									
						for(int l = 0; l < file_count; l++)
						{
							//write file names to socket
							written = write(sok, file_names[l], 
							strlen(file_names[l]));
							
							//check if write failed
							if(written < 0)
							{
								perror("Write");
								exit(EXIT_FAILURE);
							}
							
							//fail check for file names if a dir
							if ((unsigned int)written != strlen(file_names[l]))
							{
								cout << "Failed to send file names to ";
								cout << "client!" << endl;
							}
							
							//write in the new line
							written = write(sok, "\n", 1);
							
							//write check - if write failed perror
							if (written < 0)
							{
								perror("Write");
								exit(EXIT_FAILURE);
							}
							
						}
						
						//write in the new line
						written = write(sok, "\n", 1);
						
						//check if write failed and perror
						if (written < 0)
						{
							perror("Write");
							exit(EXIT_FAILURE);
						}
				}

	  	}
		
		
	}
	else if (S_ISREG(filestat.st_mode))
	{
		//normal file was given
		
		//open the file -- pointed to by total path
		int open_fd = open(tot_path, O_RDONLY);
		
		//check if open failed
		//if open failed exit
		if (open_fd < 0)
		{
			//write error to the socket
			written = write(sok, "The file cannot be opened!\n", 27);
			
			//check if write failed
			if(written <0)
			{
				perror("Write");
			}
			
			perror("Open");
			exit(EXIT_FAILURE);
		}
		
		while(true)
		{
			
			//read the file with the opened filed decriptor
			ssize_t b_read = read(open_fd, read_buf, sizeof(read_buf));
			
			//Exit condition
			if(b_read == 0)
			{
				break;
			}
			
			//check if read failed
			if (b_read < 0)
			{
				//write error to the socket
				written = write(sok, "File cannot be read!\n",21);
				
				if(written < 0)
				{
					perror("Write");
				}
							
				perror("Read");
				exit(EXIT_FAILURE);
				
			}
							
			//write to the socket
            write(sok, " \n\n", 3);
			written = write (sok, read_buf, b_read);
			
			//if the bytes that were read are not the same
			//as the ones written exit with fail and message
			if(written < 0)
			{
				perror("Write");
				exit(EXIT_FAILURE);
			}						

		}
		
	}
	else
	{
		//not a normal file or directory -- exit
		cout << "Specified field is not normal file or directory\n";
		written = write(sok, 
		"Specified field is not normal file or directory!\n", 49);
		
		//check if write failed
		if(written < 0)
		{
			//if it did report error
			perror("Write");
		}
		
		exit(EXIT_FAILURE);
	}
	
	
	
	
}
