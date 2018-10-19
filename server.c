#include <stdio.h>
#include <string.h>
#include <sys/socket.h>		/* socket() bind() connect() for the server */
#include <arpa/inet.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_REQ 5
#define MAX_ARG 500
#define MAX_SIZE 1000
#include "helper.h"


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
		exit(1);
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
	if (bind(server_socket,(struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		ShowError("binding failed");

	/*make it listen for incoming sockets*/
	if (listen(server_socket, MAX_REQ) < 0)
		ShowError("listening failed");

	while(1)
	{
		/* to fill in the client_address sockaddr after accept*/
		length_of_client_addr = sizeof(client_address);
		/*accepting the connection*/
		if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &length_of_client_addr )) < 0)
			ShowError("accepting failed");
		/*getting the ip address of client from client address sockaddr_in*/
		printf("Client %s connected \n", inet_ntoa(client_address.sin_addr));

		/* Do action here*/
		/*receive and send messages, and finally close once done. We work with the file format here*/
		int len_received = 0;
		char arguments[MAX_ARG];
		len_received = recv(client_socket, arguments, MAX_ARG, 0);
		if (len_received < 0)
		{
			ShowError("receiving failed");
		}

		printf("Arguments received");

		char f_format;
		unsigned char name_size;
		long file_size;
		int arr_pos;
		memcpy(&f_format, arguments, 1);
		arr_pos++;
		memcpy(&name_size, arguments+arr_pos,1);
		arr_pos++;
		char to_name[name_size+1];
		memcpy(to_name, arguments+arr_pos, name_size);
		arr_pos += name_size;
		to_name[name_size]='\0'; //string end with \0
		memcpy(&file_size, arguments+arr_pos, sizeof(long));

		//check if correct details is received

		printf("%d\n", f_format);
		printf("%s\n", to_name );
		printf("%ld\n", file_size );

		//send response back to server saying messages were received

		if (send(client_socket, &len_received, sizeof(int), 0)!=sizeof(int))
		{
			ShowError("sending response confirming message received failed");
		}
		printf("Confirmation successfully sent\n" );


		//create a temporary fule to store the incoming data

		FILE *temp_file_buffer = fopen("tempF.txt","wb");
		if (temp_file_buffer==NULL)
		{
			ShowError("Failed to open temp file");
		}

		char incoming_buffer[MAX_SIZE];
		long received = 0;
		long remaining = file_size;

		while (remaining > 0)
		{
			if (remaining>=MAX_SIZE)
			{
				if ((received = recv(client_socket, incoming_buffer, MAX_SIZE, 0))<0)
					ShowError("receiving failed. Allocate more size for incoming buffer");
			}
			else if ((received = recv(client_socket, incoming_buffer, remaining, 0)) < 0)
			{
				ShowError("receiving incoming buffer failed");
			}
			remaining = remaining - received;
			fwrite(incoming_buffer, received, 1, temp_file_buffer);
		}


		fclose(temp_file_buffer);

		//Now, move the file from temp to the new file with name to_name

		FILE *server_writer;
		server_writer = fopen(to_name, "wb+");
		FILE *server_reader;
		server_reader = fopen("tempF.txt","rb");	//open the temp file to read
		bool result;
		result = serve_c(server_reader, server_writer, f_format);
		remove("tempF.txt");
		char update = (char) result;
		// if (result==1)// if boolean Fail is true, ie there's a format error.
		// {
		// 	remove(to_name);
		// 	printf("removed");
		// 	update = '1';

		// }

		printf("Update is %d\n", update);

		if (send(client_socket, &update, sizeof(char), 0)!=sizeof(char))
		{
			ShowError("Update sending failed");
		}

		printf("Response sent to client\n");

		close(client_socket);

	} 


	return 0;


}