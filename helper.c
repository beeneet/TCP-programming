#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
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
long get_type_1_length(FILE *fptr, uint16_t total_commas, long file_size)
{
	int cposition = ftell(fptr);
	uint8_t current;
	long unit_length = 0;
	int no_of_commas=0;
	long prev_comma_pos = -1;
	while (true)	//get one byte each iteration and check if it is valid. 
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


void type_0_to_type_1(uint16_t *t0_units,uint8_t amount, FILE * writer)
{
	char t0_to_1[10000];
	// memset(t0_to_1,0,10000);
	int position = 0;	//it is the index of buffer, we specify the size of the contents of the units
	uint8_t type = 1;
	uint8_t cp_amount = amount;
	memcpy(t0_to_1,&type,1);	//copy the new type val on the buffer
	position ++;
	char type_1_amt[3];	//3 bytes for the ascii amount value
	int a;
	//converting the int amount to string ascii value. we know 0=48+0, 1=48+1 in ascii. 
	//start to fill the type_1_amt string from the end.
	for(a=2;a>-1;a--)
	{
		type_1_amt[a] = (cp_amount%10)+48;	//getting the last character
		cp_amount = cp_amount/10;	//reducing the end digit
	}

	memcpy(t0_to_1+position,type_1_amt,3);
	position += 3;
	/*Now we use snprintf to count the characters of the string and store the maximum no. of bytes in the buffer.
	we know, the maximum ascii value is 65535, hence we need 5 bytes for storing and 1 byte for the null character.
	*/
	char delimiter = ',';
	uint16_t num;
	for (a=0;a<amount;a++)
	{
		//extract the number
		num = (t0_units[a]<<8)|(t0_units[a]>>8);
		char num_string[6];	//Max 5, 1 for NULL
		int size = snprintf(num_string,6,"%d",num);	//max to write to num_string: 6 bytes
		//write to  t0_to_1 buffer
		memcpy(t0_to_1+position,num_string,size);
		position += size;
		//Add commas to all exceot the last one
		if (a<amount-1)
		{
			memcpy(t0_to_1+position,&delimiter,1);
			position++;
		}
	}
	fwrite(t0_to_1,sizeof(uint8_t),position,writer);
}


void type_0_to_type_0(uint8_t *type0from0, uint16_t *t0_units, uint8_t amount)
{
	//set type byte to zero
	uint8_t type = 0;
	//copy to type0from0 buffer
	int size=0;
	memcpy(type0from0+size,&type,1);
	size++;
	memcpy(type0from0+size,&amount,1);	//amount is 1 byte
	size++;
	memcpy(type0from0+size,t0_units,amount*2);
}

//just fill the type and amount of the unit. Copy t1_units till unit_length.
void type_1_to_type_1(uint8_t *type1from1, uint8_t *t1_units, char *amount, int unit_length)
{
	uint8_t type = 1;
	int size = 0;
	memcpy(type1from1+size, &type,1);
	size++;
	memcpy(type1from1+size, amount,3);
	size += 3;
	memcpy(type1from1+size, t1_units, unit_length);

}

void type_1_to_type_0(uint8_t *type0from1, uint8_t *t1_units, uint8_t amount, int unit_length)
{
	uint8_t type = 0;
	uint16_t temp[amount];
	uint16_t temp1;
	char type_1_matrix[amount][5];
	memset(type_1_matrix,32,sizeof(char)*amount*5);
	/*storing each number in a matrix. We need to traverse from the the end as it is stored that way in the matrix 
	for simplicity. It also helps with atoi() later.*/
	int string_index = unit_length -1 ;
	int prev_letter_ind;
	int a;
	int b;
	//parsing starts here
	for (a=amount-1;a>-1;a--)
	{
		for (b=4;b>-1;b--)
		{
			if (t1_units[string_index]!=',')
			{
				type_1_matrix[a][b] = t1_units[string_index];
				string_index--;
				if (string_index < 0)
				{
					prev_letter_ind = b;
					break;
				}
			}
			else 
			{
				string_index--;
				if (prev_letter_ind == 0)
				{
					a++;
				}
				prev_letter_ind = b;
				break;
			}
			prev_letter_ind = b;
		}
			
	}

	for (int a = 0; a<amount; a++)
	{
		temp1 = atoi(type_1_matrix[a]);
		temp[a] = (temp1 << 8 | temp1 >> 8);
	}

	int size = 2*(1+amount);
	memcpy(type0from1,&type,1);
	memcpy(type0from1+1,&amount,1);
	memcpy(type0from1+2, temp,amount*2);
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


void print_units_1(uint8_t *units, long unit_size)
{
	int a;
	for (a=0;a<unit_size;a++)	//amount is unit size for type 1. Print character by character
	{
		printf("%c", units[a]);
	}
	printf("\n");

}

bool serve_c(FILE *fptr, FILE* writer, int format)
{
	// FILE *fptr;
	// FILE *writer;
	uint8_t unit_type;
	// fptr = fopen("output.txt","rb");
	// fptr = fopen("practice_project_test_file_1","rb");
	// writer = fopen("output.txt","wb+");
	bool Fail = false;
	long cposition = ftell(fptr);
	fseek(fptr, 0L, SEEK_END);	//OL is the offset
	long file_size = ftell(fptr);
	fseek(fptr, cposition, SEEK_SET);	//set the position back to the original position
	uint8_t amount;
	int pos;	//for writing to a file
	long size;
	while (ftell(fptr)!=file_size)
	{
		unit_type=get_unit_type(fptr);
		// printf("Unit is %d\n",unit_type );
		if (unit_type==0)	//Testing if the program is correct for unit type 0
		{
			amount = get_type_0_amount(fptr);
			uint16_t units[amount];	//buffer to store units
			get_type_0_units(fptr, units, amount);
			print_units_0(units, amount);

			if (format == 0 || format == 2)
			{
				size = 2*(1+amount);
				uint8_t result0_0[size];
				type_0_to_type_0(result0_0, units, amount);
				fwrite(result0_0, sizeof(uint8_t),size, writer );
			}
			else if (format==1 || format==3)		//Type 0 to 1
			{
				type_0_to_type_1(units, amount, writer);	//also writes
			}

		}
		else if (unit_type==1)
		{
			char type_1_amt[3];
			uint16_t amount;
			int type_1_amt_flag = get_type_1_amount(fptr,type_1_amt);

			/* Incase the amount contains other characters except ascii, set Fail to true and break the loop.*/
			if (type_1_amt_flag <1)
			{
				Fail = true;
				break;
			}

			amount = atoi(type_1_amt);
			long type_1_length = get_type_1_length(fptr,amount-1,file_size);	//-1 since no_of_commas = total amount - 1 as last one does not end with a comma
			
			/*Incase characters except ascii are present and odd number of commas present, then break the loop*/
			if (type_1_length == -1)
			{
				Fail = true;
				break;
			}

			uint8_t units[type_1_length];
			get_type_1_units(fptr, units, type_1_length);
			print_units_1(units, type_1_length);	//printing till the end of the unit

			if (format==0 || format==1)
			{
				size = 4 + type_1_length;
				uint8_t result1_1[size];
				type_1_to_type_1(result1_1, units, type_1_amt, type_1_length);
				fwrite(result1_1, sizeof(uint8_t), size, writer);
			}
			else if (format == 2 || format == 3)		//Type 1 to 0
			{
				size = 2*(1+amount);
				uint8_t result1_0[size];
				type_1_to_type_0(result1_0, units, amount, type_1_length);
				fwrite(result1_0, sizeof(uint8_t), size, writer);
			}

		}
		else
		{
			Fail=true;	//Fail flag is set to True. Returns Failure to the client.
			// break;
		}
	}
	return Fail;
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
