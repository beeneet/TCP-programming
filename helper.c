#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "helper.h"

#define RECEIVED_MSG_SIZE 500
#define SENT_MSG_SIZE 13

void ShowError(char *err_message){
	printf("%s\n",err_message);
	exit(1);	
}

void ServeClient(int client_socket){
	char received_msg[RECEIVED_MSG_SIZE];	/*store message from client*/
	int read_msg_size;	/*size of message read*/
	char sent_msg[SENT_MSG_SIZE];
	int write_msg_size;

	if ((read_msg_size = Readline(client_socket, received_msg, RECEIVED_MSG_SIZE ))<0)
		ShowError("Read failed on server side");

	/*handle the input here */
	/* finally write it */

	if ((Writeline(client_socket, sent_msg, SENT_MSG_SIZE)) < 0)
		ShowError("Writing back to client failed");

	close(client_socket);
}

/*

  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net

  Implementation of sockets helper functions.

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/

/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		return -1;
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}
