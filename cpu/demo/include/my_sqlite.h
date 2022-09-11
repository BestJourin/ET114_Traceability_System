#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>



int date_callback(void *data, int argc, char **argv, char **azColName);
int id_count_callback(void *data, int argc, char **argv, char **azColName);
void create_table(sqlite3 *db);
int insert_item(sqlite3 *db, char **item_value);
int find_item(sqlite3 *db, int id_value);
void open_datebase(sqlite3 *db);
int get_row_count(sqlite3 *db);
int find_callback(void *data, int argc, char **argv, char **azColName);






