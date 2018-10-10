#include <stdio.h>
#include <string.h>
#include <sys/socket.h>		/* socket() bind() connect() for the server */
#include <arpa/inet.h> 
#include <stdlib.h>
#include <unistd.h>
#define MAX_REQ 5


int main(int argc, char **argv)
{
	int server_socket;	/*	socket for server 	*/
	int client_socket;	/* socket for client */
	struct sockaddr_in server_address;	/*Server address*/
	struct sockaddr_in client_address;	/*Client address*/
	unsigned short server_port;	/* port for server */
	unsigned int length_of_client_addr;	/*client address length*/

	if (argc != 2){		/* servername port, 2 arguments) */
		printf("%s <Port>\n",argv[0]);
		exit(1)
	}

	server_port = atoi(argv[1]);








}