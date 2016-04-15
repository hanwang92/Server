%{

/*
   Parser for configuration file specification of Milestone 1 - ECE297.
 */

#include <string.h>
#include <stdio.h>
#include "database.h"
#include "utils.h"

struct table *tl, *t;
int count1 = 0;
int count2 = 0;
int count3 = 0;
int count4 = 0;
int count5 = 0;
int count6 = 0;
int count7 = 0;

/* define a linked list of table names */
struct table {
  char* table_name;
  struct table *next;
};

struct table *table_list; /* first element in table name list */

/* define a structure for the configuration information */

%}
%union{
  char *sval;
  int  pval;
}
%token <sval> STRING COL COM ALPSTRING
%token <pval> PORT_NUMBER 
%token HOST_PROPERTY PORT_PROPERTY STORAGE_POLICY DDIR_PROPERTY TABLE UPASSWORD_PROPERTY UNAME_PROPERTY
%parse-param { struct config_params *params }
%%


property_list:
   haha
   haha
   haha
   haha
   haha
   haha
   haha
   {     
     params->tlist = tl; 
     int status = print_configuration(params);
	if (status == 1)
		return 1;
   } 
   ;

haha:
   host_property| port_property| uname_property| upass_property| table_list |data_directory |stor_policy
   ;

host_property:
   HOST_PROPERTY STRING
   { 
     count1++;
     if(count1==2)
	return 1;
     strcpy(params->server_host, strdup($2));
   }
   ;

port_property:
   PORT_PROPERTY PORT_NUMBER
   { 
     count2++;
     if(count2==2)
	return 1;
     params->server_port = $2;
   }
   ;

uname_property:
   UNAME_PROPERTY STRING
   { 
     count3++;
     if(count3==2)
	return 1;
     strcpy(params->username, strdup($2));
   }
   ;

upass_property:
   UPASSWORD_PROPERTY STRING
   { 
     count4++;
     if(count4==2)
	return 1;
     strcpy(params->password, strdup($2));     
   }
   ;

table_list:

   table_list TABLE STRING col_list
   { 
     count7 = 1;
     t = (struct table *) malloc(sizeof(struct table));
     if (t == NULL){
       printf("Error on malloc `table'.\n");
       exit(-1);
     }
     else{
       t->table_name = strdup($3);
	 int i = 0;
       while(params->columns[i]!=NULL)
	{	
		if(strcmp(params->columns[i],strdup($3))==0)
			return 1;
		i++;
	}
       params->columns[i] = strdup($3);
	params->columns[i+1] = "\n";
       
       t->next = tl;
       tl = t;
     }
   }
   |TABLE STRING col_list
   {
     count7 = 1;
     tl = (struct table *) malloc(sizeof(struct table));
     if (tl == NULL){
       printf("Error on malloc `table'.\n");
       exit(-1);
     }
     else{
       int i = 0;
       while(params->columns[i]!=NULL)
	{	
		if(strcmp(params->columns[i],strdup($2))==0)
			return 1;
		i++;
	}
       params->columns[i] = strdup($2);	
       params->columns[i+1] = "\n";
       tl->table_name = strdup($2);
       tl->next = NULL;
     }
   }
   ;

col_list:
   col_list COM COL
   {
	int i = 0;
       while(params->columns[i]!=NULL)
	{
		i++;
	}
	params->columns[i] = strdup($3);
   }
   | COL
   {
	int i = 0;
       while(params->columns[i]!=NULL)
	{
		i++;
	}
	params->columns[i] = strdup($1);
   }
   ;

stor_policy:
  |STORAGE_POLICY STRING '-' STRING
   {

     count5++;
     if(count5==2)
	return 1;

	if(strcmp(strdup($2),"on")!=0 && strcmp(strdup($2),"in")!=0)
		return 1;
	if(strcmp(strdup($4),"disk")!=0 && strcmp(strdup($4),"memory")!=0)
		return 1;

	if(strcmp(strdup($2),"on")==0 && strcmp(strdup($4),"disk")!=0)
		return 1;

	if(strcmp(strdup($2),"in")==0 && strcmp(strdup($4),"memory")!=0)
		return 1;
/*
	int i = (strcmp(strdup($2),"on") && strcmp(strdup($4),"disk"));
	int j = (strcmp(strdup($2),"in") && strcmp(strdup($4),"memory"));
	LOG(("%d %d", i, j));

	if(i!=0 && j!=0)
		return 1;
*/
	char temp[30];
	strcat(temp,strdup($2));
	strcat(temp,"-");
	strcat(temp,strdup($4));
       strcpy(params->storage_policy,temp);
   }
   ;

data_directory:
   |DDIR_PROPERTY STRING
   { 
     count6++;
     if(count6==2)
	return 1;
       strcpy(params->data_dir, strdup($2));
   }
   ;
%%

yyerror(char *e){
  fprintf(stderr,"%s.\n",e);
}

int print_configuration(struct config_params* params){

  if(count1==0 || count2==0 || count3==0 || count4==0 || count7 == 0)
	return 1;
  struct table *tl;

  if (strlen(params->storage_policy)==0)
	  strcpy(params->storage_policy,"in-memory");
/*
  printf("Configuration:\n Host: %s, port: %d, username: %s, password: %s, storage policy: %s, data directory: %s\n",
	 params->server_host, params->server_port, params->username, params->password, params->storage_policy, params->data_dir);

  if (tl){
    tl = params->tlist;
    printf("  Table(s):\n");
    for(;tl; tl = tl->next){
      printf("     %s ",tl->table_name);
    }
    printf("\n");
  } 
  int i = 0;
  while(params->columns[i]!=NULL)
  {
	printf("%s ",params->columns[i]);
	i++;
  }
*/
}

