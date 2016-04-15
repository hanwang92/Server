/**
 * @file
 * @brief This file contains the implementation of the storage server
 * interface as specified in storage.h.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "storage.h"
#include "utils.h"
#include <errno.h>

int authenticated = 0;

// Use the external logging constant from client.c to avoid multiple LOGGING declarations
const int LOGGING;

/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
void* storage_connect(const char *hostname, const int port)
{
    char buf[MAX_CMD_LEN];

    // check if parameters are NULL
    if((hostname == NULL) || (port == NULL))    
    {
	errno = ERR_INVALID_PARAM;
	return NULL;
    }

    // Create a socket.
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        // Log the reason for the connection failure. Useful so that user knows why connection failed. 
        logger("Connection could not be created due to failure to create socket", LOGGING, 0);
	errno = ERR_CONNECTION_FAIL;
        return NULL;
    }    
        
    // Get info about the server.
    struct addrinfo serveraddr, *res;
    memset(&serveraddr, 0, sizeof serveraddr);
    serveraddr.ai_family = AF_UNSPEC;
    serveraddr.ai_socktype = SOCK_STREAM;
    char portstr[MAX_PORT_LEN];
    snprintf(portstr, sizeof portstr, "%d", port);
    int status = getaddrinfo(hostname, portstr, &serveraddr, &res);
	
    if (status != 0) {
        // Log the reason for failure. If getaddrinfo failed it is most likely due to an incorrect hostname or server isn't running.
        sprintf(buf, "Error receiving server information. Verify that server is running, and that hostname '%s' is correct", hostname);
        logger(buf, LOGGING, 0);
	errno = ERR_CONNECTION_FAIL;
        return NULL;
    }

    // Connect to the server.
    status = connect(sock, res->ai_addr, res->ai_addrlen);

    // Log the attempted connection to the server, and the status that was received. 
    // This will give user useful information in case of connection failure, or success.
    sprintf(buf, "Attempting connection to %s:%d, received status: %d", hostname, port, status);
    logger(buf, LOGGING, 0);
        
    if (status != 0)
	{	
		errno = ERR_CONNECTION_FAIL;
        	return NULL;
	}

	return (void*) sock;
}

/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_auth(const char *username, const char *passwd, void *conn)
{
	// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	// check if parameters are NULL
	if((username == NULL) || (passwd == NULL) || (conn == NULL))    
	{
		errno = ERR_INVALID_PARAM;
		return -1;
    	}

	// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	char *encrypted_passwd = generate_encrypted_password(passwd, NULL);
	snprintf(buf, sizeof buf, "AUTH %s %s\n", username, encrypted_passwd);

    // Log authentication success or failure to inform user of either case.
	if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {

		int stat = checkreturnmessage(buf);
		if(stat==0)
		{
			// If the command was successful, log the success along with the command that was sent 
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Server authentication successful: ", 34);
			logger(strcat(buf2, buf), LOGGING, 0);
			authenticated = 1;
			return 0;
		}
		else
		{
			// If the command failed, log the failure along with the command that was sent 
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Server authentication failed: ", 29);
			logger(strcat(buf2, buf), LOGGING, 0);
			errno = stat;
			return -1;
		}
	}
    
	// If nothing is returned, then there is a connection problem with the server 
	char buf2[MAX_CMD_LEN + 25];
	strncpy(buf2, "Server connection failed: ", 29);
	logger(strcat(buf2, buf), LOGGING, 0);
	errno = ERR_CONNECTION_FAIL;
	return -1;
}

/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_get(const char *table, const char *key, struct storage_record *record, void *conn)
{
	// check if parameters are NULL
	if((table == NULL) || (key == NULL) || (conn == NULL) || (record == NULL))    
	{
		errno = ERR_INVALID_PARAM;
		return -1;
    	}
	if((strcmp(table, "") == 0) || (strcmp(key, "") == 0))    
	{
		errno = ERR_INVALID_PARAM;
		return -1;
    	}

	//check if authenticated
	if(authenticated==0)
	{
		errno = 3;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Not authenticated", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}

	//check if parameters are valid
	int validparam;
	validparam = checkinvalidparam(table, 0);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}
	validparam = checkinvalidparam(key, 0);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}

	// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	snprintf(buf, sizeof buf, "GET %s %s\n", table, key);
	
    // Log either success or failure to inform user of either case. 
    // User should know this information because if only one case is logged they may assume failure or success due to lack of info.
    if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {

		int stat = checkreturnmessage(buf);
		char rec[50];
		//char state[50];
		int count = 0;
                int rec_cnt = 0;
                int startread = 0;
               
                while(buf[count]!='\0')
                {
                        if(startread==1)
                        {
                                rec[rec_cnt] = buf[count];
                                rec_cnt++;
                        }
                        else
                        {
                                if(buf[count]==' ')
                                {
                                        startread++;
                                }
                        }
                        count++;
                }
		rec[rec_cnt] = '\0';
		if(stat==0)
		{
			// Log the command's success, along with the command itself
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Get command successful: ", 24);
			logger(strcat(buf2, buf), LOGGING, 0); 
			strncpy(record->value, rec, sizeof record->value);
			return 0;
		}
		else
		{
			// Log the command's failure, along with the command itself
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Get Command failed: ", 29);
			logger(strcat(buf2, buf), LOGGING, 0);
			errno = stat;
			return -1;
		}
	}

    // If nothing is returned, then there is a connection problem with the server 
	char buf2[MAX_CMD_LEN + 25];
	strncpy(buf2, "Server connection failed: ", 29);
	logger(strcat(buf2, buf), LOGGING, 0);
	errno = ERR_CONNECTION_FAIL;
	return -1;
}


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_set(const char *table, const char *key, struct storage_record *record, void *conn)
{
	// check if parameters are NULL
	if((table == NULL) || (key == NULL) || (conn == NULL))    
	{
		errno = ERR_INVALID_PARAM;
		return -1;
    	}
	if((strcmp(table, "") == 0) || (strcmp(key, "") == 0) )    
	{
		errno = ERR_INVALID_PARAM;
		return -1;
    	}

	//check if authenticated
	if(authenticated==0)
	{
		errno = 3;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Not authenticated: ", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}
	//check if params are valid
	int validparam;
	validparam = checkinvalidparam(table, 0);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}
	validparam = checkinvalidparam(key, 0);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}
	/*validparam = checkinvalidparam(record->value, 1);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}*/


	// Connection is really just a socket file descriptor.
	int sock = (int)conn;
            
	// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);

    // Here we are manually manipulating memory. This would be a case where the program could fail, so we log the success. 
    logger("Successfully cleared memory buffer", LOGGING, 0);

	if(record == NULL)
	{
		snprintf(buf, sizeof buf, "DEL %s %s\n", table, key);
	}
	else
	{
		if(strcmp(record->value, "") == 0)
		{
			errno = ERR_INVALID_PARAM;
			return -1;
		}
		snprintf(buf, sizeof buf, "SET %s %s %s\n", table, key, record->value);
	}
	if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {
		int stat = checkreturnmessage(buf);
		if(stat==0)
		{
			// Log the command's success, along with the command itself
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Set command successful: ", 24);
			logger(strcat(buf2, buf), LOGGING, 0);
			return 0;
		}
		else
		{
			// Log the command's failure, along with the command itself 
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Set command failed: ", 24);
			logger(strcat(buf2, buf), LOGGING, 0);
			errno = stat;
			return -1;
		}
	}

     // If nothing is returned, then there is a connection problem with the server 
	char buf2[MAX_CMD_LEN + 25];
	strncpy(buf2, "Server connection failed: ", 29);
	logger(strcat(buf2, buf), LOGGING, 0);
	errno = ERR_CONNECTION_FAIL;

	return -1;
}

int storage_query(const char *table, const char *predicates, char **keys, const int max_keys, void *conn)
 {
	//check if parameters are NULL
	if((table==NULL)||(predicates==NULL)||(conn==NULL))
	{//LOG(("asd\n"));
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}	
	
	if(keys == NULL)
	{
		if(max_keys != 0)
		{
			errno = ERR_INVALID_PARAM;
			return -1;
		}
	}
	
	if(max_keys < 0)
	{
		errno = ERR_INVALID_PARAM;
		return -1;
	}

	if((strcmp(table, "") == 0) )    
	{//LOG(("qwe %d\n",max_keys));
		errno = ERR_INVALID_PARAM;
		return -1;
    	}

	/*// case: max_keys is zero
	if(max_keys == 0)
	{LOG(("iop\n"));
		keys = NULL;
		return 0;
	}*/

	//check if parameters are valid
	int validparam;
	validparam = checkinvalidparam(table, 0);
	if(validparam == -1)
	{
		errno = 1;
		char buf2[MAX_CMD_LEN + 25];
		strncpy(buf2, "Invalid Parameter", 29);
		logger(buf2, LOGGING, 0);
		return -1;
	}	 

         // Connection is really just a socket file descriptor.
         int sock = (int)conn;
 
         // Send some data.
         char buf[MAX_CMD_LEN];
	 int cnt = 0;
         memset(buf, 0, sizeof buf);
	 if(strcmp(predicates, "") == 0)
	 {
		snprintf(buf, sizeof buf, "QUERYALL %s\n", table);
	 }
	 else
	 {
         	snprintf(buf, sizeof buf, "QUERY %s %s\n", table, predicates);
	 }
         if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {
                 int stat = checkreturnmessage(buf);
		 if(stat==0)
		{

			// Log the command's success, along with the command itself
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Query command successful: ", 24);
			logger(strcat(buf2, buf), LOGGING, 0); 

			
	
			// case: return message is "S"
			if(buf[2] == '\0')
			{
				keys = NULL;
				return 0;
			}
			
			int bufCount = 2;

			while(1)
			{
				if(cnt < max_keys)
				{
					int tempStringCount = 0;
					while(buf[bufCount] != ' ')
					{
						if(buf[bufCount] == '\0')
						{
							keys[cnt+1] = "";
							return cnt;
						}
						keys[cnt][tempStringCount] = buf[bufCount];
						bufCount++;
						tempStringCount++;					
					}
					keys[cnt][tempStringCount] = '\0';
					bufCount++;					
					cnt++;
				}
				else
				{
					keys[cnt] = "";
					while(buf[bufCount] != '\0')
					{
						if(buf[bufCount] == ' ')
						{
							cnt++;
						}
						bufCount++;					
					}
					// case: max_keys is zero
					if(max_keys == 0)
					{
						keys = NULL;
					}
					return cnt;
				}
			}
		}
		else
		{
			// Log the command's failure, along with the command itself
			char buf2[MAX_CMD_LEN + 25];
			strncpy(buf2, "Query Command failed: ", 29);
			logger(strcat(buf2, buf), LOGGING, 0);
			errno = stat;
			return -1;
		}
          }   
         return -1;
 }


/**
 * @brief This is just a minimal stub implementation.  You should modify it 
 * according to your design.
 */
int storage_disconnect(void *conn)
{
    //check invalid connection
    if(conn==NULL)
    {
	char buf2[MAX_CMD_LEN + 25];
	strncpy(buf2, "Invalid connection parameter", 29);
	logger(buf2, LOGGING, 0);
	errno = 1;
	return -1;

    }
    // Cleanup
    int sock = (int)conn;
    close(sock);

    // Log the action of closing the socket connection
    logger("Closed socket connection", LOGGING, 0);
    authenticated = 0;
    return 0;
}


