#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_SIZE 2000
#include "helper.h"


int main(int argc, char *argv[])
{
	int client_socket;	/*client socket*/
	struct sockaddr_in server_address; /*server address struct */
	unsigned short server_port;	/*server port*/
	char *server_IP;	/*server IP address x.x.x.x*/
	const char *file_details[3];	/*array to store file details ie arg[3]-arg[5]*/
	unsigned int file_details_len;
	int bytes_received, total_bytes_received;	/*bytes_received records bytes received in single read(), total_bytes_received is the total bytes received*/ 
	char *convPtr;
/*arguments check*/

	if (argc!=6){
		printf("Usage: %s <server IP> <server Port> <file path> <to format> <to name>", argv[0]);
		ShowError("Usage Error");
	}

	server_IP = argv[1];

	server_port = strtoul(argv[2],&convPtr,10);
	// server_port = argv[2];
	/*fill in the file details*/
	file_details[0] = argv[3];
	file_details[1] = argv[4];
	file_details[2] = argv[5];

	/* client socket using TCP*/

	if ((client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ShowError("client socket error");

	/* server address structure */

	memset(&server_address, 0, sizeof(server_address));	/*mem allocation*/
	server_address.sin_family = AF_INET;	/*Internet address family */
	server_address.sin_addr.s_addr = inet_addr(server_IP);	/*server IP*/
	server_address.sin_port = htons(server_port);	/*port*/

	/*connect to the server*/

	if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		ShowError("connecting to server failed");
	printf("Connected to server\n");

	//Obtaining the length of the file before sending to the server

	FILE *input_file = fopen(file_details[0],"rb");
	if (input_file==NULL)
	{
		ShowError("Failed to open file");
	}

	//Go to end and get the size using fseek.
	long cposition = ftell(input_file);
	fseek(input_file, 0L, SEEK_END);
	long file_size = ftell(input_file);
	printf("file size %ld\n", file_size);
	fseek(input_file,cposition,SEEK_SET); //go back to the beginning
	int name_length = strlen(file_details[2]);
	unsigned char to_fname;
	char f_format = atoi(file_details[1]);
	int arr_pos = 0;
	//send the data to server via a char array
	char file_details_buffer[sizeof(long)+to_fname+2];
	memcpy(file_details_buffer, &f_format,1);
	arr_pos++;
	memcpy(file_details_buffer+arr_pos, &name_length, 1);
	arr_pos++;
	memcpy(file_details_buffer+arr_pos, file_details[2], name_length);
	arr_pos += name_length;
	memcpy(file_details_buffer+arr_pos, &file_size, sizeof(long));

	//send file details to server
	long total_s = send(client_socket, file_details_buffer, arr_pos+sizeof(long),0);
	if (total_s!=arr_pos+sizeof(long))
	{
		ShowError("All file details bytes not sent");
	}
	printf("Sent file details to server\n");

	//Create a char array to send file contents to the server one by one
	char f_contents[MAX_SIZE];
	long sent = 0;
	long remaining = file_size;

	while(remaining > 0)
	{	//sending MAZ_SIZE at a time
		if (remaining > MAX_SIZE)
		{
			fread(f_contents, 1, MAX_SIZE, input_file);
			if ((sent=send(client_socket, f_contents, MAX_SIZE, 0))!=MAX_SIZE)
			{
				ShowError("sending failed");
			}
		} 
		//if remaining is less than MAX_SIZE
		else 
		{
			fread(f_contents, 1, remaining, input_file);
			if((sent=send(client_socket, f_contents, remaining, 0))!=remaining)
			{
				ShowError("sending failed");
			}
		}
		remaining = remaining - sent;	//Update remaining bytes

	}

	printf("successfully sent file to server");

	fclose(input_file);

	char update;
	long received = 0;
	while(received ==0)
	{
		if ((received = recv(client_socket, &update, 1, 0)) <= 0)
		{
			ShowError("Update failed to receive from server");
		}
		printf("Hello\n");
	}

	printf("Update is %d\n", update );
	if (update == 0)
	{
		printf("Format error\n");
	}
	else if (update == 1)
	{
		printf("Success\n");
	}

	close(client_socket);


	return 0;


}