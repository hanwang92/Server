/**
 * @file
 * @brief This file implements the storage server.
 *
 * The storage server should be named "server" and should take a single
 * command line argument that refers to the configuration file.
 * 
 * The storage server should be able to communicate with the client
 * library functions declared in storage.h and implemented in storage.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "utils.h"
#include "database.h"
#include "config_parser.tab.h"
#include <regex.h>

#define MAX_LISTENQUEUELEN 20	///< The maximum number of queued connections.
#define LOGGING 1

// Use the global server file pointer located in Utils.h. Available here for file creation on server start 
FILE *sfp;
struct config_params params;

double totprotime = 0;
struct database *db;
// The database variable to hold everything!

/**
 * @brief Insert or update a table, key, value pair to the database.
 *
 * @param table	: the table to insert or update
 * @param key	: the key to insert or update
 * @param value	: the value to insert or update
 * @return Returns 0 on success, -1 on table not found, -2 on key not found
 */
int server_set (char *table, char *key, char *column, char *value) {
	return DB_set(db, table, key, column, value, 1);
}

/**
 * @brief Process a command from the client.
 *
 * @param sock The socket connected to the client.
 * @param cmd The command received from the client.
 * @return Returns 0 on success, -1 otherwise.
 */

int process_config(char* config_file)
{
	char **r;
  	FILE *f; 
  	extern FILE *yyin;

    	f = fopen(config_file,"r");
	if (!f)
	{
	      	fprintf(stderr,"Could not open file: '%s'\n", r[1]);
	      	exit(-1);
	}
	yyin = f;
	int status = yyparse(&params);

	if(strcmp(params.storage_policy, "on-disk")==0)
		db= DB_init(params.data_dir);
	
	else if(strcmp(params.storage_policy, "in-memory")==0)
		db= DB_init();

	if(status != 0)
	{
		printf("Error processing config file.\n");
		exit(EXIT_FAILURE);
	}	

	int status2 = process_columns(0,&params);
	DB_fromDisk(db);
	fclose(f);

	if (status2!=0) 
	{
		printf("Error processing config file.\n");
		exit(EXIT_FAILURE);
	}

	return 0;

}

int process_columns(int i, struct config_params* params)
{
	if(params->columns[i]==NULL)
		return 0;
	int j = i;	

	while(strcmp(params->columns[i], "\n")!=0)
	{	i++;	}

	DB_addTable (db, params->columns[i-1]);	
	printf("adding table: %s \n",params->columns[i-1]);

	char* temp;
	char* temp2;
	char name[30];
	char type[30];
	int lala;
	char* col[15];

	while(j!=i-1)
	{	
		temp = strtok (params->columns[j],":");
		int t = 0;
 		while (temp != NULL)
  		{
			if(t==0)
			{
				strcpy(name,temp);
				int k = 0;
				while(col[k]!=NULL)
				{
					if(strcmp(col[k],name)==0)
						return 1;
					k++;
				}
				col[k] = (char*)malloc(sizeof(char*)*50);
				strcpy(col[k],name);
			}
			if(t!=0)
			{
				temp2 = strtok (temp,"char[");
				temp2 = strtok (temp2,"]");
				if(temp2==NULL)
					return 1;					
				lala=atoi(temp2);
			}
    			temp = strtok (NULL, ":");
			t++;
  		}
		DB_addColumn (db, params->columns[i-1], name, lala);
		printf("adding columns: %s, %s, %d\n", params->columns[i-1], name, lala);
		j++;
	}
	process_columns(i+1,params);
}

/**
 * @brief Process a command from the client.
 *
 * @param sock The socket connected to the client.
 * @param cmd The command received from the client.
 * @return Returns 0 on success, -1 otherwise.
 */

int handle_command(int sock, char *cmd)
{
    struct timeval start_time, end_time;
    double diff_time;
    char buf[100];
    sprintf(buf, "Processing command '%s'\n", cmd);
    logger(buf, LOGGING, 1); 
	int status;
	char cmdnum[50];
 	sscanf(cmd, "%s", cmdnum);
	if(strcmp(cmdnum, "AUTH")==0)
	{
		char username [50];
		char password [50];
		sscanf(cmd, "%s %s %s", cmdnum, username, password);

		//get start time
		gettimeofday(&start_time, NULL);

		if((strcmp(username, params.username)==0)&&(strcmp(password, params.password)==0))
		{
			char smessage[50];
			sprintf(smessage, "S");
			sendall(sock, smessage, strlen(smessage));
			sendall(sock, "\n", 1);
		}
		else
		{
			char emessage[50];
			sprintf(emessage, "E AUTH");
			sendall(sock, emessage, strlen(emessage));
			sendall(sock, "\n", 1);
		}

		//get end time and calculate difference and add to total processing time
		gettimeofday(&end_time, NULL);
		diff_time = end_time.tv_usec-start_time.tv_usec;
		diff_time = diff_time/1000000;
		totprotime += diff_time;
		LOG(("Single command processing time: %.10F seconds\nTotal processing time: %.10F seconds\n", diff_time, totprotime));
	
	}
	else if(strcmp(cmdnum, "SET")==0)
	{
		char table [50];
		char key [50];
		char record [50];
		char temp[50];
		char returnVal[100];
		int count = 0;
		int rec_cnt = 0;
		int startread = 0;
		sscanf(cmd, "%s %s %s %s\n", cmdnum, table, key, temp);
		while(cmd[count]!='\0')
		{
			if(startread==3)
			{
				record[rec_cnt] = cmd[count];
				rec_cnt++;
			}
			else
			{
				if(cmd[count]==' ')
				{
					startread++;
				}
			}
			count++;
		}
		record[rec_cnt] = '\0';
		//printf("%s %s %s %s\n", cmdnum, table, key, record);
		
		int len = strlen(record);
		//get start time
		gettimeofday(&start_time, NULL);
LOG(("HERE table: %s, key: %s, record: %s length %d\n", table, key, record, len));
		status = parse_record(db, table, key, record, returnVal, len);
LOG(("HERE2 status: %d, returnval: %s\n", status, returnVal));			
		//get end time and calculate difference and add to total processing time
		gettimeofday(&end_time, NULL);
		diff_time = end_time.tv_usec-start_time.tv_usec;
		diff_time = diff_time/1000000;
		totprotime += diff_time;
		LOG(("Single command processing time: %.10F seconds\nTotal processing time: %.10F seconds\n", diff_time, totprotime));
	

		if(status==-1)
		{	
			char emessage[50];
			sprintf(emessage, "E TABLE");
			sendall(sock, emessage, strlen(emessage));
			sendall(sock, "\n", 1);
		}
		else if(status==-2)
		{
			char emessage[50];
                        sprintf(emessage, "E KEY");
                        sendall(sock, emessage, strlen(emessage));
                        sendall(sock, "\n", 1);
		}
		else if(status==-3)
		{
			char emessage[50];
                        sprintf(emessage, "E INVALID_PARAM");
                        sendall(sock, emessage, strlen(emessage));
                        sendall(sock, "\n", 1);
		}
		else if(status==-4)
		{
			char emessage[50];
                        sprintf(emessage, "E INVALID_PARAM");
                        sendall(sock, emessage, strlen(emessage));
                        sendall(sock, "\n", 1);
		}
		else
		{	
			char smessage[50];
			sprintf(smessage, "S");
			sendall(sock, smessage, strlen(smessage));
			sendall(sock, "\n", 1);
		}
		
	}
	else if(strcmp(cmdnum, "GET")==0)
	{
		char table [50];
		char key [50];
		sscanf(cmd, "%s %s %s", cmdnum, table, key);
		//printf("%s %s %s\n", cmdnum, table, key);
		char returnval[50];

		//get start time
		gettimeofday(&start_time, NULL);
//LOG(("HERE table: %s, key: %s", table, key));
		status = DB_get(db, table, key, returnval);
		//get end time and calculate difference and add to total processing time
		gettimeofday(&end_time, NULL);
		diff_time = end_time.tv_usec-start_time.tv_usec;
		diff_time = diff_time/1000000;
		totprotime += diff_time;
		LOG(("Single command processing time: %.10F seconds\nTotal processing time: %.10F seconds\n", diff_time, totprotime));
LOG(("HERE2 status: %d, returnval: %s", status, returnval));
		if(status==-1)
		{	
			char emessage[50];
			sprintf(emessage, "E TABLE");
			sendall(sock, emessage, strlen(emessage));
			sendall(sock, "\n", 1);
		}
		else if(status==-2)
		{	
			char emessage[50];
			sprintf(emessage, "E KEY");
			sendall(sock, emessage, strlen(emessage));						
			sendall(sock, "\n", 1);
		}
		else
		{	
			char smessage[50];
			sprintf(smessage, "S %s", returnval);
			sendall(sock, smessage, strlen(smessage));
			sendall(sock, "\n", 1);
		}
	}
	else if(strcmp(cmdnum, "DEL")==0)
	{
		char table [50];
		char key [50];
		sscanf(cmd, "%s %s %s\n", cmdnum, table, key);
		//get start time
		gettimeofday(&start_time, NULL);
printf("AM I HERE\n");
		status = DB_delete(db, table, key);
	
		//get end time and calculate difference and add to total processing time
		gettimeofday(&end_time, NULL);
		diff_time = end_time.tv_usec-start_time.tv_usec;
		diff_time = diff_time/1000000;
		totprotime += diff_time;
		LOG(("Single command processing time: %.10F seconds\nTotal processing time: %.10F seconds\n", diff_time, totprotime));

		if(status==-1)
		{	
			char emessage[50];
			sprintf(emessage, "E TABLE");
			sendall(sock, emessage, strlen(emessage));
			sendall(sock, "\n", 1);
		}
		else if(status==-2)
		{
			char emessage[50];
                        sprintf(emessage, "E KEY");
                        sendall(sock, emessage, strlen(emessage));
                        sendall(sock, "\n", 1);
		}
		else
		{	
			char smessage[50];
			sprintf(smessage, "S");
			sendall(sock, smessage, strlen(smessage));
			sendall(sock, "\n", 1);
		}
		
	}

	else if(strcmp(cmdnum, "QUERY") == 0)
	{
		int stat;
		char tname[MAX_TABLE_LEN];
		char predicates [50];
		char returnstring[100];
		memset(returnstring, '\0', sizeof(returnstring));
		int count = 0;
		int rec_cnt = 0;
		int startread = 0;
		sscanf(cmd, "%s %s %s\n", cmdnum, tname, predicates);
		while(cmd[count]!='\0')
		{
			if(startread==2)
			{
				predicates[rec_cnt] = cmd[count];
				rec_cnt++;
			}
			else
			{
				if(cmd[count]==' ')
				{
					startread++;
				}
			}
			count++;
		}
		predicates[rec_cnt] = '\0';
	LOG(("OHOHOH table name: %s, predicates: %s\n", tname, predicates));
		stat = DB_query(db, tname, predicates, returnstring);
	LOG(("HOHOHO  Return String: %s\n", returnstring));

		sendall(sock, returnstring, strlen(returnstring));
		sendall(sock, "\n", 1);
	}
	else if(strcmp(cmdnum, "QUERYALL") == 0)
	{
		int stat;
		char returnstring[500];
		char tname[MAX_TABLE_LEN];
		memset(returnstring, '\0', sizeof(returnstring));
		sscanf(cmd, "%s %s\n", cmdnum, tname);

		stat = DB_queryall(db, tname, returnstring);
		LOG(("HOHOHO  Return String: %s\n", returnstring));
		
		if(stat==-1)
		{	
			char emessage[50];
			sprintf(emessage, "E TABLE");
			sendall(sock, emessage, strlen(emessage));
			sendall(sock, "\n", 1);
		}
		else if (stat == 0)
		{
			sendall(sock, returnstring, strlen(returnstring));
			sendall(sock, "\n", 1);
		}
	}
	else
	{
		char emessage[50];
		sprintf(emessage, "E PARAM");
		sendall(sock, emessage, strlen(emessage));
		sendall(sock, "\n", 1);
	}
	return 0;
}

/**
 * @brief Start the storage server.
 *
 * This is the main entry point for the storage server.  It reads the
 * configuration file, starts listening on a port, and proccesses
 * commands from clients.
 */
int main(int argc, char *argv[])
{

    // If file logging is enabled, create the appropriate server logging file
    if(LOGGING == 2) {
        char fname[255];
        time_t ti;
        struct tm * t;
        time ( &ti );
        t = localtime ( &ti );
        sprintf(fname, "Server-%d-%d-%d-%d-%d-%d.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        sfp = fopen(fname, "w");
    }

	// Process command line arguments.
	// This program expects exactly one argument: the config file name.
	assert(argc > 0);
	if (argc != 2) {
		printf("Usage %s <config_file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *config_file = argv[1];

	// Read the config file.

	int status=process_config(config_file);

    // Log info to the logger function
    char buf[100];
    sprintf(buf, "Server on %s:%d\n", params.server_host, params.server_port);
    logger(buf, LOGGING, 1);

	// Create a socket.
	int listensock = socket(PF_INET, SOCK_STREAM, 0);
	if (listensock < 0) {
		printf("Error creating socket.\n");
		exit(EXIT_FAILURE);
	}

	// Allow listening port to be reused if defunct.
	int yes = 1;
	status = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	if (status != 0) {
		printf("Error configuring socket.\n");
		exit(EXIT_FAILURE);
	}

	// Bind it to the listening port.
	struct sockaddr_in listenaddr;
	memset(&listenaddr, 0, sizeof listenaddr);
	listenaddr.sin_family = AF_INET;
	listenaddr.sin_port = htons(params.server_port);
	inet_pton(AF_INET, params.server_host, &(listenaddr.sin_addr)); // bind to local IP address
	status = bind(listensock, (struct sockaddr*) &listenaddr, sizeof listenaddr);
	if (status != 0) {
		printf("Error binding socket.\n");
		exit(EXIT_FAILURE);
	}

	// Listen for connections.
	status = listen(listensock, MAX_LISTENQUEUELEN);
	if (status != 0) {
		printf("Error listening on socket.\n");
		exit(EXIT_FAILURE);
	}

	// Listen loop.
	int wait_for_connections = 1;
	while (wait_for_connections) {
		// Wait for a connection.
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof clientaddr;
		int clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientsock < 0) {
			printf("Error accepting a connection.\n");
			exit(EXIT_FAILURE);
		}

        char buf[100];
        sprintf(buf, "Got a connection from %s:%d.\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        logger(buf, LOGGING, 1); 

		// Get commands from client.
		int wait_for_commands = 1;
		do {
			// Read a line from the client.
			char cmd[MAX_CMD_LEN];
			int status = recvline(clientsock, cmd, MAX_CMD_LEN);
			if (status != 0) {
				// Either an error occurred or the client closed the connection.
				wait_for_commands = 0;
			} else {
				// Handle the command from the client.
				int status = handle_command(clientsock, cmd);
				if (status != 0)
					wait_for_commands = 0; // Oops.  An error occured.
			}
		} while (wait_for_commands);
		
		// Close the connection with the client.
		close(clientsock);
		
        sprintf(buf, "Closed connection from %s:%d.\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        logger(buf, LOGGING, 1);
	}

	// Stop listening for connections.
	close(listensock);

	return EXIT_SUCCESS;
}


