/*
Authors: Rumeysa Tatlı, Şerifhan Işıklı
The client-side implementation of File Transfer Protocol in C
It works on linux system
$ ./client <Server IP Address> <Server Port number>

USAGE--------> 
PUT​ -> ​ PUT <filename>
GET​ -> ​ GET <filename>
LIST OF CONTENTS -> LS
CHANGE DIRECTORY -> CD
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h> 


#define MAX_LENGTH 10
#define DSK "humidity.txt"
#define PORT 4455

void put_file(int socket,char* fname);
void get_file(int socket, char* fname);
int fileExist(char *fname);
int send_file(int socket, char* fname);
int receive_file(int socket,char* fname);


int error(char *err){
	perror(err);
	exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
	// basic declaraion
	struct sockaddr_in address;
	int client_sock;
	struct sockaddr_in server_addr;
	char buffer[MAX_LENGTH];
	bzero(buffer, MAX_LENGTH);

	// socket creation
	const char* IP = argv[1];
	if (!(client_sock = socket(AF_INET, SOCK_STREAM, 0)))
		error("Failed to create a socket. Exiting! ");

	// server address configuration
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if(inet_pton(AF_INET, IP, &server_addr.sin_addr)<=0) 
		error("\nError: Invalid address. Address not supported. ");

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		error("\nError: Failed to connect to the remote host. ");

	while(1){
		bzero(buffer, MAX_LENGTH);
		fgets(buffer, MAX_LENGTH, stdin);
		buffer[strcspn(buffer, "\n")] = 0;

		// decide for the correct operation from put/get/ls/cd
		char cmd_line[MAX_LENGTH];
		strcpy(cmd_line,buffer);
		char* cmd = strtok(cmd_line," ");

		// If operation is put
		if (strcmp("PUT",cmd)==0)
		{
			// check if file exist
			char *fname;
			cmd =  strtok(NULL," ");
			fname = cmd;
			if (!fileExist(fname)){
				printf("ERROR: File %s not found.\n", fname);
				continue;
			}
			// send command to server
			printf("CLIENT : transferring %s to server.\n",buffer );
			send(client_sock, buffer, MAX_LENGTH, 0);
			put_file(client_sock, fname);
			bzero(buffer, MAX_LENGTH);
            recv(client_sock, buffer, MAX_LENGTH, 0);
            if (strcmp(buffer,"SUCCESS")){
            	printf("Error sending file. Please try again\n");
            	break;
            }
		}
		else if (strcmp("GET",cmd)==0) //if get
		{
			// check if file exist
			char *fname;
			cmd =  strtok(NULL," ");
			fname = cmd;
			// send command to server
			send(client_sock, buffer, MAX_LENGTH, 0);
			// Check for server reponse
			get_file(client_sock, fname);
		}
        else if(strcmp("CD",cmd) == 0)
		{
			//chdir(cmd + 4);
		}
        /*
        if(strncmp(user_input,"cd ",3) == 0)
		{
			sprintf(dir,"CWD %s\r\n",user_input + 3);
			send(sockfd,dir,strlen(dir),0);
		
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
			{
				message_from_server[no_of_bytes] = '\0';
				printf("%s\n",message_from_server);
				fflush(stdout);
				
				if(strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"250 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;
			}
		}
        */
        else if(strcmp("LS",cmd) == 0)
		{
            //struct dirent *de;
			//getcwd(working_dir);
			//ls_dir(working_dir);
           // while ((de = readdir(dr)) != NULL) 
            //printf("%s\n", de->d_name);
		}
		else if (strcmp("EXIT",buffer)==0)
        {
			send(client_sock, buffer, MAX_LENGTH, 0);
			break;
		}
		else
		{
			printf("Invalid Command\n");
			continue;
		}
		printf("File Transfer Protocol successful!!\n\n");
		
	}
	close(client_sock);
	return 0;
}

int fileExist(char *fname){
	/*int found = 0;
	DIR *dirr;
	struct dirent *dir;
	dirr = opendir(DSK);
	while ((dir = readdir(dirr)) != NULL){
		if(strcmp(dir->d_name,fname)==0){
			found=1;
			break;
		}
	}
	closedir(dirr);

	return found;
*/
return 0;
}

void put_file(int client_sock,char* fname){
	// Check for server reponse
	char buffer[MAX_LENGTH];
	bzero(buffer,MAX_LENGTH);
	recv(client_sock, buffer, MAX_LENGTH, 0);

	if (strcmp(buffer,"ABORT")==0)
	{
		printf("Operation aborted1.\n");
	}

	if (strcmp(buffer,"OKAY")==0)
	{
		printf("File not present in Server's Disk! \n\n");
		send_file(client_sock, fname);
		
	}
	else{
		printf("File present in Server's disk! \nDo you wish to overwrite the File %s ? (yes/no)\n",fname);
		bzero(buffer, MAX_LENGTH);
		fgets(buffer, MAX_LENGTH, stdin);
		buffer[strcspn(buffer, "\n")] = 0;
		send(client_sock, buffer, MAX_LENGTH, 0);
		if (strcmp(buffer,"yes")==0)
		{
			bzero(buffer,MAX_LENGTH);
			recv(client_sock, buffer, MAX_LENGTH, 0);
			// printf("CLIENT response OKAy %s\n",buffer );
			if (strcmp(buffer,"OKAY")==0)
			{
				send_file(client_sock, fname);
				bzero(buffer, MAX_LENGTH);
			}
			else{
				printf("Operation aborted.\n");
			}
			printf("File Transfer Protocol Completed! \n\n");
		}
		else{
			recv(client_sock, buffer, MAX_LENGTH, 0);
			if (strcmp("ABORT",buffer))
			{
				printf("Unexpected response from server\n");
			}
			printf("Operation aborted.\n\n");
		}
	}
}
void get_file(int client_sock, char* fname){
	char buffer[MAX_LENGTH];
	bzero(buffer,MAX_LENGTH);
	recv(client_sock, buffer, MAX_LENGTH, 0);

	if (strcmp(buffer,"ABORT")==0)
	{
		printf("Operation aborted1.\n");
	}
	else if (strcmp(buffer,"READY")==0)
	{
		if (fileExist(fname))
		{
			printf("Do you want to overwrite the file (yes/no) ? ");
			bzero(buffer, MAX_LENGTH);
			fgets(buffer, MAX_LENGTH, stdin);
			buffer[strcspn(buffer, "\n")] = 0;
			send(client_sock, buffer, MAX_LENGTH, 0);
			if (strcmp(buffer,"yes")==0)
			{
				
				receive_file(client_sock, fname);
				// printf("FIle can be received\n");

			}
			else{
				printf("Sending no to server\n");
				recv(client_sock, buffer, MAX_LENGTH, 0);
				if (strcmp("ABORT",buffer))
				{
					printf("Unexpected response from server\n");
				}
				printf("Operation aborted2.\n");
			}
		}
		else{
			bzero(buffer,MAX_LENGTH);
			strcpy(buffer,"yes");
			send(client_sock, buffer, MAX_LENGTH, 0);
			bzero(buffer, MAX_LENGTH);
			receive_file(client_sock, fname);
			bzero(buffer, MAX_LENGTH);
		}
	}
	else{
		printf("File %s not found.\n",fname);//Operation Aborted\n", fname);
	}
}
int send_file(int socket, char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *file = fopen(fpath, "r+");
    if(file == NULL)
    {
        printf("ERROR: File %s not found.\n", fname);
        return -1;
    }

    bzero(buffer, MAX_LENGTH); 
    int fs_block_sz; 
    while((fs_block_sz = fread(buffer, sizeof(char), MAX_LENGTH, file)) > 0){
        if(send(socket, buffer, fs_block_sz, 0) < 0){
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fname, errno);
            break;
        }
        bzero(buffer, MAX_LENGTH);
    }
    return 0;
}

int receive_file(int socket,char* fname){
	char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath,DSK);
    strcat(fpath,fname);
	FILE *out_file = fopen(fpath, "r+");
    if(out_file == NULL)
        printf("File %s Cannot create file on your pc.\n", fname);
    else{
        bzero(buffer, MAX_LENGTH); 
        int out_file_block_sz = 0;
        while((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0) {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if(write_sz < out_file_block_sz)
            {
                error("File write failed on your pc.");
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
        printf("Received the file from Server's Disk!\n\n");
        fclose(out_file); 
    }

}
