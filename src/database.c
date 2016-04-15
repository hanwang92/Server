#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <ctype.h>
#include "database.h"

/* 	DATABASE
 *	There's some documentation throughout the code, and an example usage at the bottom of the file.
 */

/* Node Functions */

struct node* node_init(char *key) {
	struct node *n = (struct node*)malloc(sizeof(struct node));
	n->next = NULL;
	n->prev = NULL;
	
    strncpy ( n->key, key, sizeof(n->key) );
	
	int i;
	for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
        memset(n->col_values[i], '\0', sizeof(n->col_values[i]));
        //strncpy(n->col_values[i], "", sizeof(""));
	}

	return n;
}

/* Linked List Functions */

int Table_ishead(struct Table *t, struct node *n) {
	if(n == t->head)
		return 1;
	else
		return 0;
}

int Table_istail(struct Table *t, struct node *n) {
	if(n == t->tail)
		return 1;
	else
		return 0;
}

/**
 * @brief 	Initialize a linked list.
 * @param 	name - the name of the table to create
 * @return	a new linked list
 */
struct Table* Table_init (char *name) {
	struct Table *t = (struct Table*)malloc(sizeof(struct Table));
    strncpy(t->name, name, sizeof(t->name));
    t->head = NULL;
    t->tail = NULL;

	int i;
	for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
        memset(t->col_names[i], '\0', sizeof(t->col_names[i]));
        //strncpy(t->col_names[i], "", sizeof(t->col_names[i]));
		t->col_string_size[i] = 0;
	}

    return t;
}

// 0 string size for int, > 0 string size for char of that size
// Return -1 if all columns are full, 0 if insert was successful
int Table_addColumn (struct Table *t, char *columnName, int col_string_size) {
	int insert_index = -1;
	int i;
	for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
        if(strlen(t->col_names[i]) == 0) {
            insert_index = i;
            break;
		}
	}
	if (insert_index == -1) {
        return -1;
	}
	else {
		strncpy(t->col_names[insert_index], columnName, strlen(columnName));
		t->col_string_size[insert_index] = col_string_size;
		return 0;	
	}
}


int Table_updateValue (struct Table *t, struct node *n, char *columnName, char *value) {
	int update_index = -1;
	int i;
	for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
		if(strcmp(t->col_names[i], columnName) == 0) {
			update_index = i;
		}
	}
	if(update_index == -1) {
		return -3;
	}
	else {
        int size = t->col_string_size[update_index];
        if(size == 0) {
            char *end;
            long num = strtol(value, &end, 10); 
            if (end == value || *end != '\0' || errno == ERANGE)
               return -4; 
        }
        else if (size > 0 && size < strlen(value)) {
            return -4; 
        }
        memset(n->col_values[update_index], '\0', sizeof(n->col_values[update_index]));
        strncpy(n->col_values[update_index], value, strlen(value));
	}
    return 0;
}

/**
 * @brief 	Append a new node to the list
 * @param 	t - the linked list to insert into
 * @param 	key - the key to use
 * @param 	value - the value to use
 * return -1 if column does not exist
 */
int Table_append (struct Table *t, char *key, char *columnName, char *value) {
    struct node *n = node_init(key);
    int ret = Table_updateValue(t, n, columnName, value);
    if (ret == 0) {
        if(t->head == NULL ) {
            n->prev = NULL;
            n->next = NULL;

            t->head = n;
            t->tail = t->head;
        }
        else {
            n->prev = t->tail;

            t->tail->next = n;
            t->tail = n;
        }
    }
    return ret;
}

/**
 *  @brief  Print an entire linked list to screen, starting from head 
 */
void Table_dump (struct Table *t) {
    printf("--- '%s' Start ---\n", t->name);
    printf("Columns\n");
    int i;
    for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
        if(strlen(t->col_names[i]) != 0) {
            printf("%s : %d\n", t->col_names[i], t->col_string_size[i]);
        }
    }
    printf("\n");
    if(t->head != NULL) {
        struct node *n = t->head;
        while(n != NULL) {
            if(Table_ishead(t, n))
                printf("HEAD ");
            if(Table_istail(t, n))
                printf("TAIL ");
            printf("\n\t%s\n", n->key);
			int i;
			for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
				if(strlen(n->col_values[i]) != 0) {
                    printf("\t\t%s : %s \n", t->col_names[i], n->col_values[i]);
                }
			}

            n = n->next;
        }
    }
    printf("--- '%s' End ---\n", t->name);
}

/*
 *  @brief  Search a linked list   
 *  @param  The key to search for 
 *  @return The found node 
 */
struct node* Table_get_node (struct Table *t, char *key) {
	struct node *curr = t->head;

	while(curr != NULL) {
		if(strcmp(curr->key, key) == 0) {
			return curr;
		}
		curr = curr->next;
	}

	return NULL;
}

/**
 * @brief 	Delete a node from a linked list
 * @param 	t - the linked list to delete from
 * @param 	key - the key of the node to be deleted
 * @return	-1 if key/node is not found. 0 on success
 */
int Table_delete (struct Table *t, char *key) {
	struct node *tbd = Table_get_node(t, key);

    if(tbd == NULL) {
        return -1;
    }
    else {
        if(t->head == tbd && t->tail == tbd) {
            free(t->head);
            t->head = NULL;
        }
        else {
            if(Table_ishead(t, tbd)) {
                t->head = tbd->next;
                t->head->prev = NULL;
            }
            else if(Table_istail(t, tbd)) {
                t->tail = tbd->prev;
                t->tail->next = NULL;
            }
            else {
                tbd->prev->next = tbd->next;
                tbd->next->prev = tbd->prev;
            }
            free(tbd);
            tbd = NULL;
	    }	
        return 0; 
	}
}

/**
 * @brief 	Insert or update a key / value pair to the linked list
 * @param 	t - the linked list to insert into or update
 * @param 	key - the key to be used
 * @param 	val - the value to be used
 * @return	-1 if key/node is not found. 0 on success
 */
int Table_set (struct Table *t, char *key, char *columnName, char *value) {
    struct node *n = Table_get_node(t, key);
    if (n == NULL) {
        return Table_append(t, key, columnName, value);
    }
    else {
		return Table_updateValue(t, n, columnName, value);
    }
}

/* Database Functions */

/**
 * @brief 	Initializes a new database struct
 * @return	the new database
 */
struct database* DB_init(char *data_directory) {
    struct database *db = (struct database*)malloc(sizeof(struct database)); 
    db->head = NULL;
    db->tail = NULL;

    memset(db->db_file, '\0', sizeof(db->db_file));
   // if(data_directory == NULL) {
     //   db->db_file = NULL;
   // }
   // else {
     if(data_directory != NULL) {
        mkdir(data_directory, 0777);
        char buf[1000];
        memset(buf, '\0', sizeof(buf));
        strncat(buf, data_directory, strlen(data_directory));
        strncat(buf, "/data.db", 8);
        strncat(db->db_file, buf, strlen(buf));
     //   db->db_file = buf;
    }
    return db;
}

void DB_toDisk(struct database *db) {
    FILE *fp = fopen(db->db_file, "w");
    if(!fp) return;

    struct Table *curr = db->head;
    while( curr != NULL ) {
        if(curr->head != NULL) {
            struct node *n = curr->head;
            while(n != NULL) {
                fwrite(curr->name, 1, strlen(curr->name), fp);
                fwrite("~", 1, 1, fp);
                fwrite(n->key, 1, strlen(n->key), fp);
                int i;
                for(i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
                    if(strlen(n->col_values[i]) != 0) {
                        fwrite("~", 1, 1, fp);
                        fwrite(curr->col_names[i], 1, strlen(curr->col_names[i]), fp);
                        fwrite("~", 1, 1, fp);
                        fwrite(n->col_values[i], 1, strlen(n->col_values[i]), fp);
                    }
                }
                fwrite("\n", 1, 1, fp);
                n = n->next;
            }
        }
        curr = curr->next;
    }
    fclose(fp);
}

void DB_fromDisk(struct database *db) {
    FILE *fp = fopen(db->db_file, "r+");
    if(!fp) return;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while((read = getline(&line, &len, fp)) != -1) {
        char *name = NULL;
        char *key = NULL;

        name = strtok(line, "~\n");
        key = strtok(NULL, "~\n");

        while(1) {
            char *col = strtok(NULL, "~\n");
            char *value = strtok(NULL, "~\n");
            if(col == NULL || value == NULL) {
                break;
            }
            else {
                DB_set(db, name, key, col, value, 0);
            }
        }
    }
    if(line) {
        free(line);
    }
    fclose(fp);
    DB_toDisk(db);
}

/**
 * @brief 	Add a table to the database
 * @param 	db - the database to insert the table into
 * @param 	tablename - the name of the table to be inserted
 */
void DB_addTable (struct database *db, char *tablename) {
    struct Table *t = Table_init(tablename);
    if(db->head == NULL ) {
        db->head = t;

        db->head->prev = NULL;
        db->head->next = NULL;

        db->tail = db->head;
    }
    else {
        t->next = NULL;
        t->prev = db->tail;
        db->tail->next = t;
        db->tail = t;
    }
}

/**
 * @brief 	Retrieve a table from the database
 * @param 	db - the database that contains the table
 * @param 	tablename - the name of the table to be returned
 * @return	the found table, or NULL if not found.
 */
struct Table* DB_getTable (struct database *db, char *tablename) {
    struct Table *curr = db->head;

    while(curr != NULL) {
        if(strcmp(curr->name, tablename) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// columnName = name of column, char_string_size = 0 for int, > 0 for column of char_string_size size
// Return -1 if all columns are full, -2 if table DNE, 0 if insert was successful.
int DB_addColumn (struct database *db, char *tablename, char *columnName, int char_string_size) {
	struct Table *t = DB_getTable(db, tablename);
    if(t == NULL) {
        return -2;
    }
    return Table_addColumn(t, columnName, char_string_size);
}

/**
 * @brief 	Set a key / value pair to a table
 * @param 	db - the database that contains the table
 * @param 	tablename - the name of the table insert into
 * @param 	key - the key to insert or update
 * @param 	value - the value to insert or update
 * @return	-1 if table not found, -2 if key not found, 0 on success
 */
int DB_set (struct database *db, char *tablename, char *key, char *columnName, char *value, int persist) {
    struct Table *t = DB_getTable(db, tablename);
    if(t == NULL) { 
        return -1;
    }
    else {
        int stat = Table_set(t, key, columnName, value);
        if(stat == 0 && persist == 1) {
            DB_toDisk(db);
        }
        return stat;
    }
}

/*
 * @brief   Delete a node from the given database	
 * @param 	db - the database that contains the table
 * @param 	tablename - the name of the table insert into
 * @param 	key - the key to delete 
 * @return	-1 if table not found, -2 if key not found, 0 on success
 */
int DB_delete (struct database *db, char *tablename, char *key) {
    struct Table *t = DB_getTable(db, tablename);
    if(t == NULL) {
        return -1;
    }
    else {
        if(Table_delete(t, key) == -1) {
			return -2;
		}
    }
    DB_toDisk(db);
	return 0;
}

/*
 * @brief   Return a value from the database	
 * @param 	db - the database that contains the table
 * @param 	tablename - the name of the table insert into
 * @param   key - the key associated with the value to be returned
 * @param   returnVal - the string pointer for the found string to be stored in 
 * @return	-1 if table not found, -2 if key not found, 0 on success
 */
int DB_get (struct database *db, char *tablename, char *key, char *returnVal) {
    struct Table *t = DB_getTable(db, tablename);
    if(t == NULL) {
		return -1;
    }
	else {
        struct node *n = Table_get_node(t, key);
        if(n == NULL) {
			return -2;
		}
		else {
			char content[MAX_VALUE_LEN];
			memset(content, '\0', sizeof(content));
            memset(returnVal, '\0', sizeof(returnVal));
            int i;
			for (i = 0; i < MAX_COLUMNS_PER_TABLE; i++) {
				if(strlen(n->col_values[i]) > 0) {
					if(strlen(content) > 0)
						strncat(content, " , ", 3);
                    strncat(content, t->col_names[i], strlen(t->col_names[i]));
                    strncat(content, " ", 1);
                    strncat(content, n->col_values[i], strlen(n->col_values[i]));
			    }
            }
			strncat(returnVal, content, strlen(content));
            return 0;
        }
    }
}

/*********** I Added four functions below for checking the predicate and implmenting the query function -Han ***************/

/**** Starts here **********/ 
int parse_record(struct database *db, char *tablename, char* key, char* the_value, char *returnString, int len)
{
	struct Table *t = DB_getTable(db, tablename);
    	if(t == NULL) 
    	{
		strcpy(returnString, "E TABLE");
		return -1;
    	}
printf("the value %s\n",  the_value);
	int predicate_cnt = 0;
	int col_per_table = 0;
	int stat;
    
	// parse predicate string and store in a data structure
	char predicate_col_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1];
	char predicate_col_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1];

	// set to NULL

	int init2 = 0;
	
	while(init2 < MAX_COLUMNS_PER_TABLE)
	{
		memset(predicate_col_name[init2], '\0', sizeof(predicate_col_name[init2]));
		memset(predicate_col_value[init2], '\0', sizeof(predicate_col_value[init2]));
		init2++;
	}

	// the bulk
	//bool invalid_predicate = true;
	while(the_value[predicate_cnt] != '\0')
	{
		int col_name_cnt = 0;
		int col_value_cnt = 0;
		while(the_value[predicate_cnt] == ' ')
		{
			predicate_cnt++;
		}
		while((the_value[predicate_cnt] != ' ')&&(the_value[predicate_cnt] != '\0'))
		{		
			if(isalnum(the_value[predicate_cnt]) == 0)
			{
				strcpy(returnString, "E INVALID_PARAM");
				return -3;
			}
					
			predicate_col_name[col_per_table][col_name_cnt] = the_value[predicate_cnt];
			predicate_cnt++;
			col_name_cnt++;
		}

		while(the_value[predicate_cnt] == ' ')
		{
			predicate_cnt++;
		}
		while((the_value[predicate_cnt] != ',')&&(the_value[predicate_cnt] != '\0'))
		{
			if(the_value[predicate_cnt] == '+') 
			{
				predicate_cnt++;
			}
			if(the_value[predicate_cnt] == '-')
			{
				;
			}
			else if(isalnum(the_value[predicate_cnt]) == 0)
			{
				if(the_value[predicate_cnt] != ' ')
				{
					strcpy(returnString, "E INVALID_PARAM");
					return -3;
				}
			}				
			predicate_col_value[col_per_table][col_value_cnt] = the_value[predicate_cnt];
			predicate_cnt++;
			col_value_cnt++;
		}

		int length = (strlen(predicate_col_value[col_per_table])-1);

		while(predicate_col_value[col_per_table][length] == ' ')
		{
			predicate_col_value[col_per_table][length] = '\0';
			length -= 1;
		}

		if(the_value[predicate_cnt] == ',')
		{
			predicate_cnt++;
		}
		col_per_table++;
	}

	// check for matching cases
	stat = check_record(db, t, key, predicate_col_name, predicate_col_value, returnString);
	return stat;
		
	
}

int check_record(struct database *db, struct Table* theTable, char* key, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1], char*return_string)
{

int i = 0;
while(predicate_name[i][0] != '\0')
{
	printf("Column %d: %s\n Value %d: %s\n", i, predicate_name[i], i, predicate_value[i]);
	i++;
}

	//check if out of order
	int pn_cnt = 0;
	int cn_cnt = 0;
	while(predicate_name[pn_cnt][0] != '\0')
	{
		while(theTable->col_names[cn_cnt][0] != '\0')
		{
			if(strcmp(predicate_name[pn_cnt], theTable->col_names[cn_cnt]) == 0)
			{
				cn_cnt++;
				break;
			}
			cn_cnt++;
		}
		if(theTable->col_names[cn_cnt][0] == '\0')
		{
			pn_cnt++;
			if(predicate_name[pn_cnt][0] != '\0')
			{
				strcpy(return_string, "E INVALID_PARAM");
				return -3;
			}
			if (predicate_name[pn_cnt][0] == '\0')
			{
				if(strcmp(predicate_name[pn_cnt-1], theTable->col_names[cn_cnt-1]) != 0)
				{
					strcpy(return_string, "E INVALID_PARAM");
					return -3;
				}
			}
		}
		pn_cnt++;
	}

	int predicate_names_cnt = 0;
	return_string[0] = 'S';
	return_string[1] = '\0';
	int found = 0;
	while(predicate_name[predicate_names_cnt][0] != '\0')
	{
printf("WHAT IS THE PREDICATE NAME? :%s\n",predicate_name[predicate_names_cnt] );
		int col_names_cnt = 0;
		while(theTable->col_names[col_names_cnt][0] != '\0')
		{
printf("WHAT IS THE COL NAME? :%s\n",theTable->col_names[col_names_cnt] );
			if(strcmp(theTable->col_names[col_names_cnt], predicate_name[predicate_names_cnt]) == 0)
			{printf("AM HERE\n");
				found = 1;
				if(theTable->col_string_size[col_names_cnt] == 0)
				{
					int value_cnt = 0;
					if((predicate_value[predicate_names_cnt][value_cnt] == '+') ||(predicate_value[predicate_names_cnt][value_cnt] == '-'))
					{
						value_cnt++;
					}
					while(predicate_value[predicate_names_cnt][value_cnt] != '\0')
					{printf("AM HERE pnamecnt is %s\n", predicate_value[predicate_names_cnt]);
						if(isdigit(predicate_value[predicate_names_cnt][value_cnt]) == 0)
						{printf("AM HERE IS WRONG:%c:\n", predicate_value[predicate_names_cnt][value_cnt]);
							strcpy(return_string, "E INVALID_PARAM");
							return -3;
						}
						value_cnt++;
					}
					
				}

				
				if(theTable->col_string_size[col_names_cnt] != 0)
				{	
					/*char tempstring[theTable->col_string_size[col_names_cnt]+1];
					int tempcnt = 0;
					while (tempcnt < theTable->col_string_size[col_names_cnt])
					{
						tempstring[tempcnt] = predicate_value[predicate_names_cnt][tempcnt];
					
					}*/
					
				}

				/*if(stat != 0)
				{
					return stat;
				}*/
			}
			col_names_cnt++;
		}
		if(found == 0)
		{
			strcpy(return_string, "E INVALID_PARAM");
			return -3;
		}
		found = 0;
		predicate_names_cnt++;
	}
	
	int stat = set_all(db, theTable, key, predicate_name, predicate_value);	
	return stat;
}

int set_all(struct database *db, struct Table* theTable, char* key, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1])
{
	int p_cnt = 0;
	int stat;
	while(predicate_name[p_cnt][0] != '\0')
	{
		printf("DO I COME HERE?? key: %s predicatename: %s predicatevalue: %s\n", key, predicate_name[p_cnt], predicate_value[p_cnt]);
		stat = DB_set(db, theTable->name, key, predicate_name[p_cnt], predicate_value[p_cnt], 1);
		p_cnt++;
		if(stat != 0)
		{
			return stat;
		}
	}
	return 0;
}

int DB_query (struct database *db, char *tablename, char *predicates, char *returnString)
{
    	struct Table *t = DB_getTable(db, tablename);
    	if(t == NULL) 
    	{
		strcpy(returnString, "E TABLE");
		return -1;
    	}

	int predicate_cnt = 0;
	int col_per_table = 0;
	int stat;
    	while(1)
    	{
		// parse predicate string and store in a data structure
		char predicate_col_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1];
		char operator[MAX_COLUMNS_PER_TABLE];
		char predicate_col_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1];

		// initialize to NULL
		int init2 = 0;
		while(init2 < MAX_COLUMNS_PER_TABLE)
		{
			memset(predicate_col_name[init2], '\0', sizeof(predicate_col_name[init2]));
			memset(predicate_col_value[init2], '\0', sizeof(predicate_col_value[init2]));
			init2++;
		}
		memset(operator, '\0', sizeof(operator));

		// the bulk
		//bool invalid_predicate = true;
		while(predicates[predicate_cnt] != '\0')
		{
			int col_name_cnt = 0;
			int col_value_cnt = 0;
			while(predicates[predicate_cnt] == ' ')
			{
				predicate_cnt++;
			}
			while((predicates[predicate_cnt] != '<')&&(predicates[predicate_cnt] != '>')&&(predicates[predicate_cnt] != '=')&&(predicates[predicate_cnt] != '\0'))
			{	
				if(isalnum(predicates[predicate_cnt]) == 0)
				{
					if(predicates[predicate_cnt] != ' ')
					{
						strcpy(returnString, "E INVALID_PARAM");
						return -3;
					}
				}			
				predicate_col_name[col_per_table][col_name_cnt] = predicates[predicate_cnt];
				predicate_cnt++;
				col_name_cnt++;
			}
			
			//delete trailing spaces
			int length = (strlen(predicate_col_name[col_per_table])-1);
			while(predicate_col_name[col_per_table][length] == ' ')
			{
				predicate_col_name[col_per_table][length] = '\0';
				length -= 1;
			}
		
			operator[col_per_table] = predicates[predicate_cnt];
			predicate_cnt++;
			while(predicates[predicate_cnt] == ' ')
			{
				predicate_cnt++;
			}
			while((predicates[predicate_cnt] != ',')&&(predicates[predicate_cnt] != '\0'))
			{
				if(predicates[predicate_cnt] == '+') 
				{
					predicate_cnt++;
				}
				if(predicates[predicate_cnt] == '-')
				{
					;
				}
				else if(isalnum(predicates[predicate_cnt]) == 0)
				{
					if(predicates[predicate_cnt] != ' ')
					{
						strcpy(returnString, "E INVALID_PARAM");
						return -3;
					}
					strcpy(returnString, "E INVALID_PARAM");
					return -3;
				}				
				predicate_col_value[col_per_table][col_value_cnt] = predicates[predicate_cnt];
				predicate_cnt++;
				col_value_cnt++;
			}

			//delete trailing spaces
			int length2 = (strlen(predicate_col_value[col_per_table])-1);
			while(predicate_col_value[col_per_table][length2] == ' ')
			{
				predicate_col_value[col_per_table][length2] = '\0';
				length2 -= 1;
			}

			if(predicates[predicate_cnt] == ',')
			{
				predicate_cnt++;
			}
			col_per_table++;
		}

int i = 0;
while(predicate_col_name[i][0] != '\0')
{
	printf("Column %d: %s\n Operator %d: %c\n Value %d: %s\n", i, predicate_col_name[i], i, operator[i], i, predicate_col_value[i]);
	i++;
}		

		// check for matching cases
		stat = check_predicate(t, predicate_col_name, operator, predicate_col_value, returnString);
		return stat;
		
	}
}

int check_predicate(struct Table* theTable, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char* operation, char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1], char* return_string )
{
	int predicate_names_cnt = 0;
	return_string[0] = 'S';
	return_string[1] = ' ';
	int rs_cnt = 2;
	int found = 0;
	int loop = 0;
	int count = 0;
	int written[MAX_COLUMNS_PER_TABLE];

	/*//check if out of order
	int pn_cnt = 0;
	int cn_cnt = 0;
	while(predicate_name[pn_cnt][0] != '\0')
	{
		while(theTable->col_names[cn_cnt][0] != '\0')
		{
			if(strcmp(predicate_name[pn_cnt], theTable->col_names[cn_cnt]) == 0)
			{
				cn_cnt++;
				break;
			}
			cn_cnt++;
		}
		if(theTable->col_names[cn_cnt][0] == '\0')
		{
			pn_cnt++;
			if(predicate_name[pn_cnt][0] != '\0')
			{
				strcpy(return_string, "E INVALID_PARAM");
				return -3;
			}
			if (predicate_name[pn_cnt][0] == '\0')
			{
				if(strcmp(predicate_name[pn_cnt-1], theTable->col_names[cn_cnt-1]) != 0)
				{
					strcpy(return_string, "E INVALID_PARAM");
					return -3;
				}
			}
		}
		pn_cnt++;
	}*/

	//check if int has spaces

	for(loop = 0; loop < MAX_COLUMNS_PER_TABLE; loop++)
	{	written[loop] = -2;}
	//memset(written, -2, sizeof(written));
	printf("%d %d %d %d \n\n\n\n", written[0], written[1], written[2],written[3]);
	//bool predicates_exist = true;
	while(predicate_name[predicate_names_cnt][0] != '\0')
	{
printf("WHAT IS THE PREDICATE NAME? :%s\n",predicate_name[predicate_names_cnt] );
		int col_names_cnt = 0;
		while(theTable->col_names[col_names_cnt][0] != '\0')
		{
printf("WHAT IS THE COL NAME? :%s\n",theTable->col_names[col_names_cnt] );
			if(strcmp(theTable->col_names[col_names_cnt], predicate_name[predicate_names_cnt]) == 0)
			{
				
				if(operation[predicate_names_cnt] == '=')
				{found = 1;
printf("OPERATOR =\n");
					// traverse through all keys to find matching ones
					struct node* tempptr = theTable->head;
					count = 0;
					while(tempptr != '\0')
					{
						if(strcmp(tempptr->col_values[col_names_cnt], predicate_value[predicate_names_cnt]) == 0)
						{
							
							
							if(written[count] == -2)
							{
								written[count] = count;
							}
						}
						else
						{
							written[count] = -1;
						}
						count++;
						tempptr = tempptr->next;
					}
				}
				else if((operation[predicate_names_cnt] == '<') || (operation[predicate_names_cnt] == '>'))
				{	found = 1;
printf("OPERATOR < OR >\n");
					// check if not an integer
					if(theTable->col_string_size[col_names_cnt] > 0)
					{
						strcpy(return_string, "E INVALID_PARAM");
						return -3;
					}
					
					//check if not int
					int value_cnt = 0;
					if((predicate_value[predicate_names_cnt][value_cnt] == '+') ||(predicate_value[predicate_names_cnt][value_cnt] == '-'))
					{
						value_cnt++;
					}
					while(predicate_value[predicate_names_cnt][value_cnt] != '\0')
					{
						if(isdigit(predicate_value[predicate_names_cnt][value_cnt]) == 0)
						{
							strcpy(return_string, "E INVALID_PARAM");
							return -3;
						}
						value_cnt++;
					}
					
					// traverse through all keys to find matching ones
					struct node* tempptr = theTable->head;
					count = 0;
					while(tempptr != '\0')
					{
						if(operation[predicate_names_cnt] == '<')
						{
							int one = atoi(tempptr->col_values[col_names_cnt]);
							int two = atoi(predicate_value[predicate_names_cnt]);
							if(one < two)
							{
							
								if(written[count] == -2)
								{
									written[count] = count;
								}
							}
							else
							{
								written[count] = -1;
							}
						}
						else if(operation[predicate_names_cnt] == '>')
						{printf("OPERATOR >\n");
							int one = atoi(tempptr->col_values[col_names_cnt]);
							int two = atoi(predicate_value[predicate_names_cnt]);
							printf("ONE IS {%d} TWO IS {%d}\n", one, two);
							if(one > two)
							{
						
							printf("WRITTEN IS {%d}\n", written[count]);
								if(written[count] == -2)
								{
									written[count] = count;
								}
							printf("SECOND WRITTEN IS {%d}\n", written[count]);
							}
							else
							{
								written[count] = -1;
							}
						
						}
						printf("RETURN STRING SO FAR: %s\n", return_string);
						tempptr = tempptr->next;
						count++;
					}
				}
				else
				{
					strcpy(return_string, "E INVALID_PARAM");
					return -3;
				}
				break;
			}
	
			col_names_cnt++;
		}
		if(found == 0)
		{
			strcpy(return_string, "E INVALID_PARAM");
			return -3;
		}
		found = 0;
		
		predicate_names_cnt++;
	}

	/*if(strlen(return_string) <= 3)
	{
		strcpy(return_string, "E KEY");
		return -2;
	}*/

	int lol = 0;
		while(lol < MAX_COLUMNS_PER_TABLE)
		{printf("here? %d\n", written[lol]);
			
			if((written[lol] != -1)&&(written[lol] != -2))
			{printf("WRITTEN LOL IS {%d}\n", written[lol]);
				int i;
				struct node* tempptr = theTable->head;
				for(i=0; i<written[lol]; i++)
					tempptr = tempptr->next;
				int key_cnt = 0;
				while(tempptr->key[key_cnt] != '\0')
				{
					return_string[rs_cnt] = tempptr->key[key_cnt];
					rs_cnt++;
					key_cnt++;
				}							
				return_string[rs_cnt] = ' ';
				rs_cnt++;
			}
			lol++;
		}
	printf("RET STR IS {%s}\n", return_string);
	return 0;
}

int DB_queryall (struct database *db, char *tablename, char *returnString)
{
    	struct Table *t = DB_getTable(db, tablename);
    	if(t == NULL) 
    	{
		strcpy(returnString, "E TABLE");
		return -1;
    	}
	returnString[0] = 'S';
	returnString[1] = ' ';
	int count = 2;
	int col_names_cnt = 0;
	struct node* temp = t->head;
	while(temp != '\0')
	{
		int key_cnt = 0;
		while(temp->key[key_cnt] != '\0')
		{
			returnString[count] = temp->key[key_cnt];
			count++;
			key_cnt++;
		}	
		returnString[count] = ' ';
		count++;
		temp = temp -> next;	
	}
	returnString[count] = '\0';
	return 0;
}

/******** Ends here ********************/

/*
 * @brief   Print the entire database to the screen
 * @param   db - the database to print
 */
void DB_dump (struct database *db) {
    struct Table *curr = db->head;
    printf("** START DB DUMP **\n");
    while( curr != NULL ) {
        Table_dump(curr);
        curr = curr->next;
    }
    printf("** END DB DUMP **\n");
}
/*
// An example function to show some usage
int DB_TEST(char *f) {
    struct database *db = DB_init(f);
	DB_addTable(db, "school");
	DB_addColumn(db, "school", "mark", 0);
    DB_addColumn(db, "school", "gender", 1);
    DB_addColumn(db, "school", "part_time", 0);
    DB_addColumn(db, "school", "age", 0);
    DB_addColumn(db, "school", "citizenship", 20);

    DB_addTable(db, "teams");
    DB_addColumn(db, "teams", "goals", 0);
    DB_addColumn(db, "teams", "sport", 20);

    DB_addTable(db, "cities");
    DB_addColumn(db, "cities", "avg_temp", 0);
    DB_addColumn(db, "cities", "population", 0);
    DB_addColumn(db, "cities", "province", 50);

//    DB_set(db, "school", "han", "citizenship", "this is a citizensp", 1);
    DB_fromDisk(db);
    //DB_set(db, "teams", "toronto", "goals", "82", 1);
    DB_dump(db);

    DB_set(db, "school", "jordan", "marks", "100", 1); 
    DB_set(db, "school", "jordan", "gender", "M", 1);
    DB_set(db, "school", "jordan", "citizenship", "canadian", 1);
    DB_set(db, "school", "jordan", "part_time", "1", 1);

    DB_set(db, "school", "xiang", "marks", "100", 1);
    DB_set(db, "school", "xiang", "gender", "F", 1);
    DB_set(db, "school", "xiang", "part_time", "0", 1);

    DB_set(db, "teams", "toronto", "goals", "10", 1);
    DB_set(db, "teams", "toronto", "sport", "hockey", 1);
    DB_set(db, "teams", "detroit", "goals", "50", 1);
    DB_set(db, "teams", "buffalo", "goals", "20", 1);

    DB_set(db, "cities", "toronto", "avg_temp", "5", 1);
    DB_set(db, "cities", "winnipeg","avg_temp",  "0", 1);
    DB_set(db, "cities", "nunavut", "avg_temp", "-10");
    DB_set(db, "cities", "winnipeg", "population", "2000000", 1);
    DB_set(db, "cities", "toronto", "province", "ontario", 1);

    char buf[1000];
    memset(buf, '\0', sizeof(buf));
    DB_get(db, "school", "han", buf);
    if(strlen(buf) > 0)
        printf("%s\n", buf);
//	printf("returning to main\n");
    return 0;
}

int main(int argc, char* argv[]) {
	DB_TEST(argv[1]);	
	return 0;
}
*/
