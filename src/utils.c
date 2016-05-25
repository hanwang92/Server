/**
 * @file
 * @brief This file implements various utility functions that are
 * can be used by the storage server and client library. 
 */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include "utils.h"

// Use the global file pointers (declared in utils.h) to log both client and server information
FILE *cfp;
FILE *sfp;

int counter1 = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;

int sendall(const int sock, const char *buf, const size_t len)
{
	size_t tosend = len;
	while (tosend > 0) {
		ssize_t bytes = send(sock, buf, tosend, 0);
		if (bytes <= 0) 
			break; // send() was not successful, so stop.
		tosend -= (size_t) bytes;
		buf += bytes;
	};

	return tosend == 0 ? 0 : -1;
}

/**
 * In order to avoid reading more than a line from the stream,
 * this function only reads one byte at a time.  This is very
 * inefficient, and you are free to optimize it or implement your
 * own function.
 */
int recvline(const int sock, char *buf, const size_t buflen)
{
	int status = 0; // Return status.
	size_t bufleft = buflen;

	while (bufleft > 1) {
		// Read one byte from scoket.
		ssize_t bytes = recv(sock, buf, 1, 0);
		if (bytes <= 0) {
			// recv() was not successful, so stop.
			status = -1;
			break;
		} else if (*buf == '\n') {
			// Found end of line, so stop.
			*buf = 0; // Replace end of line with a null terminator.
			status = 0;
			break;
		} else {
			// Keep going.
			bufleft -= 1;
			buf += 1;
		}
	}
	*buf = 0; // add null terminator in case it's not already there.

	return status;
}


/**
 * @brief Parse and process a line in the config file.
 */

int process_config_line(char *line, struct config_params *params)
{
	if (line[0] == CONFIG_COMMENT_CHAR)
		return 0;

	// Extract config parameter name and value.
	char name[MAX_CONFIG_LINE_LEN];
	char value[MAX_CONFIG_LINE_LEN];

	char temp_char[MAX_CONFIG_LINE_LEN];
	int temp_int[MAX_CONFIG_LINE_LEN];

	int items = sscanf(line, "%s %s\n", name, value);
	int int_item =  sscanf(line, "%s %d\n", temp_char,temp_int);

	// Line wasn't as expected.
	if (items != 2)
		return -1;

	// Process this line.
	if (strcmp(name, "server_host") == 0) {
		strncpy(params->server_host, value, sizeof params->server_host);
		counter1++;
	} else if (strcmp(name, "server_port") == 0) {
		if(int_item != 2)
			return -1;
		params->server_port = atoi(value);
		counter2++;
	} else if (strcmp(name, "username") == 0) {
		strncpy(params->username, value, sizeof params->username);
		counter3++;
	} else if (strcmp(name, "password") == 0) {
		strncpy(params->password, value, sizeof params->password);
		counter4++;
	}else if(strcmp(name, "table") == 0){
	//check duplicate table name		
		int i = 0;
		while(params->table[i] != NULL)
		{
			if(strcmp(params->table[i],value)==0)
			{
				counter1 = 2;
				return -1;
			}
			i++;
		}
		params->table[counter5] = (char*)malloc(sizeof(char));
		strcpy(params->table[counter5], value);
		params->table[counter5+1] = NULL;
		counter5++;
	}
	return 0;
}


int read_config(const char *config_file, struct config_params *params)
{
	int error_occurred = 0;

	// Open file for reading.
	FILE *file = fopen(config_file, "r");
	if (file == NULL)
		error_occurred = 1;

	// Process the config file.
	while (!error_occurred && !feof(file)) {
		// Read a line from the file.
		char line[MAX_CONFIG_LINE_LEN];
		char *l = fgets(line, sizeof line, file);

		// Process the line.
		if (l == line)
			process_config_line(line, params);
		else if(counter1 != 1 || counter2 != 1 || counter3 != 1 || counter4 != 1)
			error_occurred = 1;
		else if (!feof(file))
			error_occurred = 1;
	}
	return error_occurred ? -1 : 0;
}

/*
 *  logger   : log messages from both client and server side
 *  message  : the message to log
 *  logging  : constant to determine whether to log to stdout or the respective client and server files
 *  is_server: a bit indicator determining whether log command comes from server or client side  
 */
void logger(char *message, int logging, int is_server) {

    // Create the prefix including the time for all logging messages
    char prefix[255];
    time_t ti;
    struct tm * t;
    time ( &ti );
    t = localtime ( &ti );
    sprintf(prefix, "LOG -> %d-%d-%d-%d-%d-%d :: ", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    // IF logging == 1 log messages to the stdout
    if (logging == 1) {
        // Server messages come with a newline, so just output message
        if (is_server == 1) {
            printf("%sSERVER :: %s", prefix, message);
        }
        // Client messages come without a newline, so append a newline character
        else {
            printf("%sCLIENT :: %s\n", prefix, message);
        }
    }
    // If logging == 2 log messages to a file and flush file to ensure immediate update of file
    else if (logging == 2) {
        // Log to the server file
        if (is_server == 1) {
            fputs(prefix, sfp);
            fputs(message, sfp);
            fflush(sfp);
        } 
        // Log to the client file
        else {
            fputs(prefix, cfp);
            fputs(message, cfp);
            fputs("\n", cfp);
            fflush(cfp);
        }
    }
} 

char *generate_encrypted_password(const char *passwd, const char *salt)
{
	if(salt != NULL)
		return crypt(passwd, salt);
	else
		return crypt(passwd, DEFAULT_CRYPT_SALT);
}

int checkreturnmessage(char* message)
{
	char state[50];
	sscanf(message, "%s", state);
	if(strcmp(state, "S")==0)
	{
		return 0;
	}
	else if(strcmp(state, "E")==0)
	{
		char errorcode[50];
		sscanf(message, "%s %s", state, errorcode);
		if(strcmp(errorcode, "INVALID_PARAM")==0)
		{
			return 1;
		}
		if(strcmp(errorcode, "ERR_CONNECTION_FAIL")==0)
		{
			return 2;
		}
		if(strcmp(errorcode, "ERR_NOT_AUTHENTICATED")==0)
		{
			return 3;
		}
		if(strcmp(errorcode, "AUTH")==0)
		{
		
			return 4;
		}
		if(strcmp(errorcode, "TABLE")==0)
		{
		
			return 5;
		}
		if(strcmp(errorcode, "KEY")==0)
		{
			
			return 6;
		}
		if(strcmp(errorcode, "ERR_UNKNOWN")==0)
		{
			
			return 7;
		}
		if(strcmp(errorcode, "ERR_TRANSACTION_ABORT")==0)
		{
			
			return 8;
		}
	}
	return -1;
}

int checkinvalidparam(const char* cmdline, int detectspace)
{
	int count = 0;
	if(detectspace==0)
	{
		while(cmdline[count]!='\0')
		{
			if(isalnum(cmdline[count])==0)
			{
				return -1;
			}
			count++;
		}
	}
	else if(detectspace==1)
	{
		while(cmdline[count]!='\0')
		{
			if(cmdline[count]==' ')
			{
				;
			}
			else if(isalnum(cmdline[count])==0)	
			{
				return -1;
			}
			count++;
		}
	}
	return 0;	
}

