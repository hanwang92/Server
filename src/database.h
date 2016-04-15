#include "storage.h"

/* Node Stuct -> holds a key value pair */
struct node {
    struct node *next;
    struct node *prev;

    char key[MAX_KEY_LEN];
	char col_values[MAX_COLUMNS_PER_TABLE][MAX_VALUE_LEN + 1];
};

/* Linked List Struct -> holds one table (in form of linked list of nodes) and links to   next / previous table */
struct Table {
    char name[MAX_TABLE_LEN];
	char col_names[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN + 1];
	int col_string_size[MAX_COLUMNS_PER_TABLE];

    struct node *head;
    struct node *tail;

    struct Table *next;
    struct Table *prev;
};

/* Database Struct -> holds the tables in a form of a linked list */
struct database {
    char db_file[1000];
    struct Table *head;
    struct Table *tail;
};

struct node* node_init(char *key);

int Table_ishead(struct Table *t, struct node *n);

int Table_istail(struct Table *t, struct node *n);

struct Table* Table_init (char *name);

int Table_addColumn(struct Table *t, char *columnName, int col_string_size);

int Table_updateValue (struct Table *t, struct node *n, char *columnName, char *value);

int Table_append (struct Table *t, char *key, char *columnName, char *value);

void Table_dump (struct Table *t);

struct node* Table_get_node (struct Table *t, char *key);

int Table_delete (struct Table *t, char *key);

int Table_set (struct Table *t, char *key, char *columnName, char *value);

struct database* DB_init();

void DB_addTable (struct database *db, char *tablename);

struct Table* DB_getTable (struct database *db, char *tablename);

int DB_set (struct database *db, char *tablename, char *columnName, char *key, char *value, int persist);

int DB_delete (struct database *db, char *tablename, char *key);

int DB_get (struct database *db, char *tablename, char *key, char *returnVal);

void DB_dump (struct database *db);

int DB_TEST();

int parse_record(struct database *db, char *tablename, char* key, char* the_value, char *returnString, int len);

int check_record(struct database *db, struct Table* theTable, char* key, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1], char*return_string);

int DB_query (struct database *db, char *tablename, char *predicates, char *returnString);

int check_predicate(struct Table* theTable, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char* operation, char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1], char* return_string );

int set_all(struct database *db, struct Table* theTable, char* key, char predicate_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN+1], char predicate_value[MAX_COLUMNS_PER_TABLE][MAX_STRTYPE_SIZE+1]);

int DB_queryall (struct database *db, char *tablename, char *returnString);



