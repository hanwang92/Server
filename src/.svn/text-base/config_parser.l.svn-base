%{
/*
   table cities lowTemperature:int,highTemperature:int,province:char[20]
   Lexer for configuration file specification of Milestone 1 - ECE297.
 */

#include "config_parser.tab.h"
#include <string.h>
%}
%option noyywrap

/* Define macros for use below */
a2Z  [a-zA-Z./]
lala [a-zA-Z0-9./]
host server_host
port server_port 
uname username
upass password
dir data_directory
pol storage_policy
col (?:([a-zA-Z0-9]+):((char[\[0-9\]]+)|(int)))

%%

{host}		 { 
		   return HOST_PROPERTY;
		 }

{port}		 { 
		   return PORT_PROPERTY;
		 }

{uname}		 { 
		   return UNAME_PROPERTY;
		 }

{upass}		 { 
		   return UPASSWORD_PROPERTY;
		 }

table		 { 
		   return TABLE;
		 }

{pol}		 {
		   return STORAGE_POLICY;
		}

{dir}		 { 
		   return DDIR_PROPERTY;
		 }

[\t\n ]+         { /* ignore white space */
                 }
 
#.*\n	         { /* ignore comments; text starting with '#' to 
                      the end of the line */
                  }

                 
{a2Z}{lala}*	 { /* probably a string value */
                   yylval.sval = (char *)strdup(yytext);
		   return STRING;
                 }
               
[0-9]+           { /* probably the port number */
                   yylval.pval = (int) atoi(yytext);
		   return PORT_NUMBER;
                  }

{col}*		 { /* probably a column value */
                   yylval.sval = (char *)strdup(yytext);
		   return COL;
                 }

[,]		{
		   return COM;
		}
                 
.                 { 
                   return yytext[0]; 
                  }

<<EOF>>           { 
                    return 0;
                  }
%%

