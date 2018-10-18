#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

/*arguments check*/

	if (argc!=6){
		printf("Usage: %s <server IP> <server Port> <file path> <to format> <to name>", argv[0]);
		ShowError("Usage Error");
	}

	server_IP = argv[1]
	server_port = argv[2]
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

	



}