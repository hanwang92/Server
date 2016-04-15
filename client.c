/**
 * @file
 * @brief This file implements a "very" simple sample client.
 * 
 * The client connects to the server, running at SERVERHOST:SERVERPORT
 * and performs a number of storage_* operations. If there are errors,
 * the client exists.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "storage.h"
#include "utils.h"

// Declare and initialize a global logging constant to be used by both client.c and storage.c
extern const int LOGGING; 
const int LOGGING = 0;
struct timeval start_time, end_time;
double diff_time;
int errno;

// Use the client file pointer declared in Utils.h . Available here for file creation.
FILE *cfp; 

// A helper function which sets the connection to null on disconnect.   
int disconnect_from_server(void **conn) {
    int status = storage_disconnect(*conn);
    *conn = NULL;
    return status;
}


// Main Method
int main(int argc, char *argv[]) {
  // Declare variables to hold connection, status, and record information  
  void *conn;
  int status;
  struct storage_record r;
 
  // Set connection to NULL in order to keep track of connection validity
  conn = NULL;

  // If file logging is enabled, create the appropriate client logging file
  if(LOGGING == 2) {
      // Initialize the client logging file
      char fname[255];
      time_t ti;
      struct tm * t;
      time ( &ti );
      t = localtime ( &ti );
      sprintf(fname, "Client-%d-%d-%d-%d-%d-%d.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
      cfp = fopen(fname, "w");
  }

  logger("---Started client---", LOGGING, 0); 
  
  // Always loop. Break on exit.
  while (1) {

    // Output the main menu, and obtain input.
    printf("\n--- Main Menu. Choose one option from below ---\n1) Connect\n2) Authenticate\n3) Set\n4) Get\n5) Disconnect\n6) Exit\n\n"); 
    char line[MAX_CMD_LEN];
    if(fgets(line, sizeof(line), stdin) == NULL) {
      // Stop execution if fgets fails (throwing program into an infinite loop) 
      logger("Failure with fgets that would cause infinite loop.", LOGGING, 0);
      printf("ERROR %d\n", errno);
      return 0;
    }
    line[strlen(line) - 1] = '\0';

    // Connect 
    if( strcmp(line,"1") == 0 ) {
      printf("--- Connect To Server ---\n");

      // Obtain user input for hostname and port
      char hostname[MAX_CMD_LEN];
      char str[MAX_CMD_LEN]; 
      int port;

      printf("Please enter the hostname: ");
      fgets(hostname, sizeof(hostname), stdin);
      hostname[strlen(hostname) - 1] = '\0';
      printf("Please enter the port: ");
      fgets(str, sizeof(str), stdin);
      sscanf(str, "%d", &port);

      // Log the user's input for initializing the connection
      char buf[MAX_CMD_LEN];
      sprintf(buf, "Received user input for connection: %s:%d", hostname, port);
      logger(buf, LOGGING, 0);

      // Connect to server

	//get start time
	gettimeofday(&start_time, NULL);

      conn = storage_connect(hostname, port);
      if(!conn) {
        printf("Cannot connect to server @ %s:%d. Error code: %d.\n", hostname, port, errno);

	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      } 
	else {
        printf("Server connection succesful!\n");

	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      }
    }

    // Exit Program
    else if( strcmp(line,"6") == 0 ) {
      if(conn != NULL ) {
        disconnect_from_server(&conn);
      }
      
      logger("--- Exiting Program ---", LOGGING, 0);  
      printf("--- Exiting Program ---\n");

      if(LOGGING == 2) { 
          fclose(cfp);
      }
      return 0;
    }

    // Check for NULL connection
    else if (conn == NULL ) {
       printf ("No valid connection yet. Please initialize a connection first\n");
    }  

    // Authenticate
    else if( strcmp(line,"2") == 0) {
      printf("--- Authenticate ---\n");

      // Obtain user input for username and password
      char username[MAX_CMD_LEN];
      char passwd[MAX_CMD_LEN];
      char* encryptedpasswd;

      printf("Please enter your username: ");
      fgets(username, sizeof(username), stdin);
      username[strlen(username) - 1] = '\0';
      printf("Please enter your password: ");
      fgets(passwd, sizeof(passwd), stdin);
      passwd[strlen(passwd) - 1] = '\0';
       encryptedpasswd = generate_encrypted_password(passwd, NULL);


      // Log the information
      char buf[MAX_CMD_LEN];
      sprintf(buf, "User credentials received.  username '%s' and password '%s'", username, encryptedpasswd);
      logger(buf, LOGGING, 0);

	//get start time
	gettimeofday(&start_time, NULL);

      // Authenticate the client.
      status = storage_auth(username, passwd, conn);
      if(status != 0) {
        printf("storage authentication failed. Check log for more information.\n"); 
        //disconnect_from_server(&conn);
	
	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      }
      else {
        printf("storage_auth: successful.\n");
	
	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      }
    }

    // Set
    else if( strcmp(line,"3") == 0 ) {
      printf("--- Set ---\n");

      // Obtain user input for table name and key
      char table[MAX_CMD_LEN];
      char key[MAX_CMD_LEN];
      char value[MAX_CMD_LEN];

      printf("Please enter the table name: ");
      fgets(table, sizeof(table), stdin);
      table[strlen(table) - 1] = '\0';
      printf("Please enter the key: ");
      fgets(key, sizeof(key), stdin);
      key[strlen(key) - 1] = '\0'; 
      printf("Please enter the value: ");
      fgets(value, sizeof(value), stdin);
      value[strlen(value) - 1] = '\0'; 

      // Log that the information was received by the client program
      char buf[MAX_CMD_LEN];
      sprintf(buf, "Received user input for set command. Table: %s, Key: %s, Value: %s", table, key, value);
      logger(buf, LOGGING, 0);
      
	//get start time
	gettimeofday(&start_time, NULL);

      // Issue storage_set
      strncpy(r.value, value, sizeof r.value);
      status = storage_set(table, key, &r, conn);
      if(status != 0) {
        printf("Set command failed. Check the log for more information.\n");
        //disconnect_from_server(&conn);

	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      }
      else {
        printf("storage_set: successful.\n");

	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      } 
    }

    // Get
    else if( strcmp(line,"4") == 0 ) {
      printf("--- Get ---\n");
      
      // Obtain user input for table name and key
      char table[MAX_CMD_LEN];
      char key[MAX_CMD_LEN];

      printf("Please enter the table name: ");
      fgets(table, sizeof(table), stdin);
      table[strlen(table) - 1] = '\0';
      printf("Please enter the key: ");
      fgets(key, sizeof(key), stdin);
      key[strlen(key) - 1] = '\0';

      // Log that the information was received
      char buf[MAX_CMD_LEN];
      sprintf(buf, "Received user input for set command. Table: %s, Key: %s", table, key);
      logger(buf, LOGGING, 0);
 
	//get start time
	gettimeofday(&start_time, NULL);

      // Issue storage_get
      status = storage_get(table, key, &r, conn);
      if(status != 0) {
        printf("Storage get failed. Check the log for more information.\n");
        //disconnect_from_server(&conn);
	errno = 5;
	
	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      } 
      else {
        printf("storage_get: the value returned for key '%s' is '%s'.\n",
           key, r.value);
	
	//get end time and calculate difference
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_usec-start_time.tv_usec;
	diff_time = diff_time/1000000;
	printf("Processing time: %.10F seconds\n", diff_time);
      }
    }

    // Disconnect
    else if( strcmp(line,"5") == 0 ) {
      printf("--- Disconnect ---\n");

      // Disconnect from server
      status = disconnect_from_server(&conn);
      
      if(status != 0) {
        // Log if the disconnect failed
        char buf[MAX_CMD_LEN];
        sprintf(buf, "storage_disconnect failed. Error code: %d.\n", errno);
        logger(buf, LOGGING, 0);
        printf(buf);
      }
    }

    // Incorrect Value
    else {
      printf("Please enter a correct value!\n");
    }
  }
}

