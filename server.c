#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define SIZE1 2048

int lis_fd, comm_fd, port_no;
struct sockaddr_in serverAddress, clientAddress;
pthread_t id;
struct in_addr inAddr;
struct hostent * host;
char *filename;
char file_path[SIZE1], buf[SIZE1];
char hostip[32];
char file_not_found[] = "HTTP/1.0 404 Not Found\n";
char file_ok[] = "HTTP/1.0 200 OK\n";
FILE *fileptr;
int isGet;

void *handlerFunc(void *);
void init(int, char**);
void printInfo();
void fileNameExtraction(char[]);

int main(int argc, char** argv) {
	init(argc, argv);
	lis_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero( &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
	serverAddress.sin_port = htons(port_no);
	
	bind(lis_fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	listen(lis_fd, 10);
	int len = sizeof(clientAddress);
	while(1) {
		printf("Listening...\n");
		comm_fd = accept(lis_fd, (struct sockaddr*)&clientAddress, &len);
		printInfo();
		if(pthread_create(&id, NULL, handlerFunc, (void *)&comm_fd) < 0) {
			printf("Thread is not created\n");
			return 1;
		}
	}
	return 0;
}


/**
	prints client information like ip and hostname
*/
void printInfo() {
	inet_ntop(AF_INET, &(clientAddress.sin_addr), hostip, 32);
	inet_pton(AF_INET, hostip, &inAddr);
	host = gethostbyaddr(&inAddr, sizeof(inAddr), AF_INET);
	printf("**** Connection received from: %s [IP = %s] ****\n", host->h_name, hostip);	
}

/**
	initializes the variables
*/
void init(int argc, char** argv) {
	if(argc < 1) {
		printf("INVALID REQUEST\n");
		printf("usage: myserver port_number\n");
		exit(1);		
	}
	port_no = atoi(argv[1]);
	if (port_no > 65536 || port_no < 5000) {
		printf("Invalid Port Number, enter port between 5001 to 65536");
		exit(1);
	}	
}

/**
	function for handling of thread tasks
*/

void* handlerFunc(void* clientSock) {
	char str[SIZE1], dir[SIZE1], temp[SIZE1];
	int comm_fd = *(int*)clientSock, ret;
	bzero(str, 100);
	ret = recv(comm_fd, str, SIZE1, 0);
	if(ret <= 0)
		return 0;
	for(int i = 0; i < 2048; i++)
		temp[i] = str[i];
	fileNameExtraction(temp);
	getcwd(dir, sizeof(dir));
	sprintf(file_path, "%s/%s", dir, filename);	
	printf("\n--------------------------------------------------------------------------------\n");
	if(isGet == 1) {	// send the file to user
		printf("Client is downloading the file ...\n");
		if ((fileptr = fopen(file_path, "r")) == NULL ) {
			printf("File not found!\n");
			send(comm_fd, file_not_found, strlen(file_not_found), 0);	// sends HTTP 404 response
			return 0;
		}
		send(comm_fd, file_ok, strlen(file_ok), 0);	// sending HTTP 200 OK response
		memset(&buf, 0, sizeof(buf));
		while(!feof(fileptr)) {	// read from file until eof is not encountered				
			fread(&buf, sizeof(buf), 1, fileptr);	// read from file
			// printf("%s\n", buffer);
			send(comm_fd, buf, sizeof(buf), 0);	// send to client
			memset(&buf, 0, sizeof(buf));
		}
		printf("File download completed ...\n");
	}
	else {	// upload the file on own system
		printf("UPLOADING FILE in progress ...\n");
		send(comm_fd, file_ok, strlen(file_ok), 0);
		fileptr = fopen(file_path, "w");	// variable set by main method
		// printf("%s\n", file_path);
		if (fileptr == NULL) {
			printf("Error opening file!\n");
			return 0;
		}
		memset(&buf, 0, sizeof(buf));
		while(recv(comm_fd, buf, SIZE1, 0) > 0) { //receives the file
			fprintf(fileptr, "%s", buf);
			// printf("%s\n", buffer);
			memset(&buf, 0, sizeof(buf));
		}
		fclose(fileptr);
		printf("UPLOADING FILE completed ...\n");
	}
	printf("\n--------------------------------------------------------------------------------\n");
	close(comm_fd);
	return 0;
}

/**
	extracts file name from request and also set isGet variable to identify GET and PUT requests
*/
void fileNameExtraction(char str[]) {
	filename = strtok(str, " ");
	if(strcmp(filename, "GET") == 0)
		isGet = 1;
	else
		isGet = 0;
	filename = strtok(NULL, " ");
	filename += 1;	// to remove '/' from '/index.html'
}