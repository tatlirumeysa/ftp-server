/*
Authors: Rumeysa Tatlı, Şerifhan Işıklı
The server implementation of File Transfer Protocol in C
It works on linux system
$ ​./server <Server Port number>
$ gcc server.c -o server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h> 

#define MAX_LENGTH 10
#define MAX_CONN 1
#define PORT 4455
#define DSK "humidity.txt"


void put_file(int socket,char* fname);
void get_file(int socket, char* fname);
int send_file(int socket, char* fname);
int receive_file(int socket,char* fname);

int error(char *err){
	perror(err);
	exit(EXIT_FAILURE);
}

FILE *myfile;

int main(int argc, char const *argv[])
{

	// Basic Declarations for socket programming
	int server_sock;
	struct sockaddr_in client_addr;
	char buffer[MAX_LENGTH];
	bzero(buffer, MAX_LENGTH);

	// Create a socket for communication
	if (!(server_sock = socket(AF_INET, SOCK_STREAM, 0)))
		error("Failed to create a socket. Exiting!");

	// configure client address details
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(PORT);

	// Bind the socket to the application
	if (bind(server_sock,(struct sockaddr*)&client_addr,(socklen_t)sizeof(client_addr)) < 0)
		error("Binding failed. Exiting!");

	// Start listening to the port
	if (listen(server_sock, MAX_CONN)<0)
		error("Unable to listen to the port");

	// printf("fo\n");
	
	// Create a loop for accepting connections
	while(1){
		int accept_sockfd;
		int addr_len = sizeof(client_addr);
        printf("Awaiting Connection..\n");

		if( (accept_sockfd = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len)) < 0)
			error("accept");
        printf("Connection received\n");
		
		while(1){
            
			// receive command from user
			recv(accept_sockfd, buffer, MAX_LENGTH, 0);
			// printf("%s",buffer );
			// if (buffer)
			// {
			// 	break;
			// }
			char cmd_line[MAX_LENGTH];
			strcpy(cmd_line,buffer);
			char* cmd = strtok(cmd_line," ");
			printf("%s\n",cmd );
			if (strcmp(cmd,"EXIT")==0){
				printf("Exiting\n");
				break;
			}
			else if (strcmp(cmd,"GET")==0)
			{

                char *fname;
                cmd =  strtok(NULL," ");
                fname = cmd;
                get_file(accept_sockfd, fname);
                
  			}
			else if (strcmp(cmd,"PUT")==0)
			{
				char *fname;
				cmd =  strtok(NULL," ");
				fname = cmd;
                put_file(accept_sockfd, fname);
                bzero(buffer,MAX_LENGTH);
                strcpy(buffer,"SUCCESS");
                send(accept_sockfd, buffer, MAX_LENGTH, 0);
				
			}
            else if(strcmp(cmd, "LS")==0) {
                system("ls > /tmp/ls.txt");
                // read into a buffer to be sent
                myfile=fopen("/tmp/ls.txt","r");
                status = fread(buffer, sizeof(buffer), sizeof(char), myfile);
                fclose(myfile);
            }
            else if(strcmp(cmd, "CD")==0) {
                chdir(cmd);
                memset(cmd, '\0', sizeof(cmd));
                /* in order to move backwards in a directory */
            }

			else{
				printf("\nInvalid\n");
			}
			
			bzero(buffer, MAX_LENGTH);
		}
				

    	close(accept_sockfd);
        printf("Connection closed\n");
		
	}
	close(server_sock);


	return 0;
}

void put_file(int accept_sockfd,char* fname){
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    strcpy(buffer,"OKAY");
    send(accept_sockfd,buffer,MAX_LENGTH,0);
    receive_file(accept_sockfd,fname);
}

void get_file(int accept_sockfd, char* fname){
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    strcpy(buffer,"READY");
    send(accept_sockfd,buffer,MAX_LENGTH,0);
    recv(accept_sockfd, buffer, MAX_LENGTH,0);
    send_file(accept_sockfd, fname);
}

int receive_file(int socket,char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *out_file = fopen(fpath, "r+");
    if(out_file == NULL)
        printf("File %s Cannot create file on server.\n", fname);
    else
    {
        bzero(buffer, MAX_LENGTH); 
        int out_file_block_sz = 0;
        while((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0) 
        {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if(write_sz < out_file_block_sz)
            {
                error("File write failed on server.");
            }
            
            bzero(buffer, MAX_LENGTH);
            if (out_file_block_sz == 0 || out_file_block_sz != MAX_LENGTH ) 
            {
                break;
            }
        }
        if(out_file_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                printf("recv() timed out.\n");
            }
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                exit(1);
            }
        }
        printf("Received file from Client!\n\n");
        fclose(out_file); 
    }

}

int send_file(int socket, char* fname){
    char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
    FILE *file = fopen(fpath, "r");
    if(file == NULL)
    {
        printf("ERROR: File %s not found.\n", fname);
        return -1;
    }

    bzero(buffer, MAX_LENGTH); 
    int fs_block_sz; 
    while((fs_block_sz = fread(buffer, sizeof(char), MAX_LENGTH, file)) > 0)
    {
        if(send(socket, buffer, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fname, errno);
            break;
        }
        bzero(buffer, MAX_LENGTH);
    }
    return 0;

}
