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

	server_port = atoi(argv[1]); /*conver port from string to short*/

	/* server socket for incoming connections */
	if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ShowError("socket error");

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;	/*Internet addr family */
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);	/*converts host byte order to network byte order */
	server_address.sin_port = htons(server_port);

	/*Bind*/
	if (bin(server_socket,(struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		ShowError("binding failed")

	/*make it listen for incoming sockets*/

	if (listen(server_socket, MAX_REQ) < 0)
		ShowError("listening failed")

	while(1)
	{
		/* to fill in the client_address sockaddr after accept*/
		length_of_client_addr = sizeof(client_address);

		/*accepting the connection*/
		if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &length_of_client_addr )) < 0)
			ShowError("accepting failed")
		/*getting the ip address of client from client address sockaddr_in*/
		printf("Client %s connected \n", inet_ntoa(client_address.sin_addr));

		/* Do action here*/
		/*receive and send messages, and finally close once done. We work with the file format here*/
	} 







}