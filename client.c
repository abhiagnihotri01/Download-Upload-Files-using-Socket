#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SIZE1 2048

int sock_fd, port_no;
char receiveData[SIZE1];
struct sockaddr_in serverAddress;
char *url, *filename;
char req[SIZE1], directory[SIZE1], buf[SIZE1], file_path[SIZE1];
int isGet;  // 1 => GET, 0 => PUT
FILE *ptr;	// used to point file while uploading and downloading


void init(int, char**);	// instantiates the variables
void fileWrite();

int main(int argc, char **argv) {
	init(argc, argv);	// sets all the required variables
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serverAddress, sizeof serverAddress);
 
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(url);
	serverAddress.sin_port = htons(port_no);
 
	connect(sock_fd,(struct sockaddr *)&serverAddress,sizeof(serverAddress)); 
	send(sock_fd, req, strlen(req)+1, 0);	// sends the request
	getcwd(directory, sizeof(directory));	// gets the current directory
	sprintf(file_path, "%s/%s", directory, filename);	
	recv(sock_fd, receiveData, SIZE1, 0);	// response for the request	
	if(isGet == 1) {
		// READING the file now
		printf("Response: %s\n", receiveData);	// response from server
		if((strstr(receiveData, "Not")) != NULL)
			return 0;
		fileWrite();
		printf("File successfully downloaded.\n");
	}
	else {	// upload the file
		// printf("%s\n", filepath);
		if ((ptr = fopen(file_path, "r")) == NULL ) {
			printf("File not found!\n");
			return 0;
		}
		memset(&buf, 0, sizeof(buf));
		while(!feof(ptr)) {	// read from file until eof is not encountered				
			fread(&buf, sizeof(buf), 1, ptr);
			// printf("%s\n", buffer);
			send(sock_fd, buf, sizeof(buf), 0);
			memset(&buf, 0, sizeof(buf));
		}
		printf("File successfully uploaded.\n");
	}
	close(sock_fd);
	return 0;
}

/**
	writes the data read from the server on GET request to the file specified
*/
void fileWrite() {
	ptr = fopen(file_path, "w");	// variable set by main method
	if (ptr == NULL) {
		printf("Error opening file!\n");
		return;
	}
	memset(&buf, 0, sizeof(buf));
	while(recv(sock_fd, buf, SIZE1, 0) > 0) { //receives the file
		// fwrite(&buffer, sizeof(buffer), 1, fileptr);
		fprintf(ptr, "%s", buf);
		memset(&buf, 0, sizeof(buf));
	}
	fclose(ptr);
	
}


/**
	sets some basic variables
*/
void init(int argc, char **argv) {
	if(argc < 5) {
		printf("INVALID REQUEST\n");
		printf("usage: myclient host port_number GET/PUT filename\n");
		exit(1);
	}
	url = argv[1];
	port_no = atoi(argv[2]);
	if (port_no > 65536 || port_no < 5000) {
		printf("Invalid Port Number, enter port between 5001 to 65536");
		exit(1);
	}
	if(strcmp(argv[3], "GET") == 0) {
		isGet = 1;
	}
	else if(strcmp(argv[3], "PUT") == 0) {
		isGet = 0;
	}
	else {
		printf("INVALID REQUEST\n");
		printf("reason: only GET or PUT requests are allowed.\n");
		exit(1);
	}
	filename = argv[4];
	if(isGet) {
		sprintf(req, "GET /%s HTTP/1.0", filename);
	}
	else {
		sprintf(req, "PUT /%s HTTP/1.0", filename);
	}	
}