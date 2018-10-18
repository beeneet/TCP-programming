#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "helper.h"

#define RECEIVED_MSG_SIZE 500
#define SENT_MSG_SIZE 13

//unit is only a byte, hence uint8_t
uint8_t get_unit_type(FILE *fptr)
{
	return getc(fptr);
}

uint8_t get_type_0_amount(FILE *fptr)
{
	return getc(fptr);	//returns the amount of type 0, which is 1 byte.
}

int get_type_1_amount(FILE *fptr, char*type_1_amt)
{
	fread(type_1_amt, sizeof(char), 3, fptr);	//Read 3 bytes one at a time, type_1_amt contains the 3 byte content of the buffer
	int a;
	for (a=0;a<3;a++)
	{
		if (type_1_amt[a] < 48 && type_1_amt[a]>57)	//if the ascii digit does not fall on the 0-9 range, then return a flag
		{
			return -1;
		}
	}
	return 1;	//return 1 to know the amount is valud
}

//each value is 2 bytes, hence uint16_t for units
void get_type_0_units(FILE *fptr, uint16_t *units, uint8_t amount)
{
	fread(units, sizeof(uint16_t), amount, fptr); //reads 2 bytes from the file for amount times.
}

//using fseek() and ftell() to figure out the length. fseek() points to a given offset of a filestream using offset and ftell shows the length
int get_type_1_length(FILE *fptr, uint8_t total_commas)
{
	int cposition = ftell(fptr);
	fseek(fptr, 0L, SEEK_END);	//OL is the offset
	int file_size = ftell(fptr);
	fseek(fptr, cposition, SEEK_SET);	//set the position back to the original position
	uint8_t current;
	int unit_length = 0;
	int no_of_commas=0;
	long prev_comma_pos = -1;
	while (1)	//get one byte each iteration and check if it is valid. 
	{
		current = fgetc(fptr);
		long cpos = ftell(fptr);
		//check if this type has reached at the end. Check if the commas are complete
		if ((no_of_commas==total_commas)&&(current==1||current==0))
		{
			fseek(fptr, -1, SEEK_CUR);	//taking it back one byte of the curr pointer and breaking the loop
			break;
		}
		else if (current<48&&current>57)
		{
			return -1;
		}
		else if (current==',') //Update the commas count and record previous comma instance
		{	
			if (cpos-1==prev_comma_pos)	//2 commas
			{	
				return -1;
			}
			prev_comma_pos = cpos;	//curr becomes prev comma
			no_of_commas ++;	//update number of commas
		}
		unit_length++;	//increase unit length
		if (cpos==file_size)	//if end of file is reached, break
		{
			break;
		}
	}
	fseek(fptr,cposition,SEEK_SET);	//set it back where it was before calling this function
	return unit_length;
}

//reading 1 byte at a time, hence use uint8_t.
void get_type_1_units(FILE *fptr, uint8_t *units, uint8_t amount)
{
	fread(units,sizeof(uint8_t), amount, fptr);	//units variable contains the ASCII buffer
}

//two different functions created for print(for checking) because of difference in the data type of units. uint16_t vs uint8_t
void print_units_0(uint16_t *units, uint8_t amount)
{
	int a;
	for (a=0;a<amount;a++)
	{
		uint16_t number = (units[a] << 8) | (units[a] >> 8);
		printf("%d ",number );
	}
}


void print_units_1(uint8_t *units, int unit_size)
{
	int a;
	for (a=0;a<unit_size;a++)	//amount is unit size for type 1. Print character by character
	{
		printf("%c", units[a]);
	}
	printf("\n");

}





int main()
{
	FILE *fptr;
	uint8_t unit_type;
	fptr = fopen("practice_project_test_file_2","rb");
	bool Fail = false;
	int cposition = ftell(fptr);
	fseek(fptr, 0L, SEEK_END);	//OL is the offset
	int file_size = ftell(fptr);
	fseek(fptr, cposition, SEEK_SET);	//set the position back to the original position
	uint8_t amount;
	while (ftell(fptr)!=file_size)
	{
		unit_type=get_unit_type(fptr);
		// printf("Unit is %d\n",unit_type );
		if (unit_type==0)	//Testing if the program is correct for unit type 0
		{
			amount = get_type_0_amount(fptr);
			printf("The amount is %d \n", amount );
			uint16_t units[amount];	//buffer to store units
			get_type_0_units(fptr, units, amount);
			print_units_0(units, amount);
		}
		else if (unit_type==1)
		{
			printf("yes");
			char type_1_amt[3];
			uint16_t amount;
			int type_1_amt_flag = get_type_1_amount(fptr,type_1_amt);
			amount = atoi(type_1_amt);
			printf("The amount is %d \n", amount );
			int type_1_length = get_type_1_length(fptr,amount-1);	//-1 since no_of_commas = total amount - 1 as last one does not end with a comma
			uint8_t units[type_1_length];
			get_type_1_units(fptr, units, amount);
			print_units_1(units, type_1_length);	//printing till the end of the unit

		}
		else
		{
			Fail=true;	//Fail flag is set to True. Returns Failure to the client.
			// break;
		}
	}




}


































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
/* unline recv() &  send(), Readline() and Writeline() can be used for UDP sockets too*/
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
