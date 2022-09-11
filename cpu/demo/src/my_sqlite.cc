#include "my_sqlite.h"
#include <string.h>
#include<iostream>
int id_num = 0;
int item_value[9];
bool find_flag = 0;
bool count_flag = 0;
std::string D_ID_Worker_0, D_ID_Worker_1, D_ID_Worker_2, D_ID_Worker_3, D_ID;
int date_callback(void *data, int argc, char **argv, char **azColName)
{
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int id_count_callback(void *data, int argc, char **argv, char **azColName)
{
   count_flag = 1;
   fprintf(stderr, "%s: ", (const char*)data);
   id_num = std::atoi(argv[0]);
   printf("\nID_number:::%d\n",id_num);
   printf("\n");
   return 0;
}

int find_callback(void *data, int argc, char **argv, char **azColName)
{
   find_flag = 1;
   fprintf(stderr, "%s: ", (const char*)data);
   for(int i=0; i<9;i++){
       item_value[i] = 0;
   }
   D_ID = argv[0];
   item_value[1]=std::atoi(argv[1]);
   item_value[2]=std::atoi(argv[2]);
   D_ID_Worker_0=argv[3];
   item_value[3]=std::atoi(argv[4]);
   item_value[4]=std::atoi(argv[5]);
   item_value[5]=std::atoi(argv[6]);
   D_ID_Worker_1=argv[7];
   item_value[6]=std::atoi(argv[8]);
   item_value[7]=std::atoi(argv[9]);
   item_value[8]=std::atoi(argv[10]);
   D_ID_Worker_2=argv[11];
   D_ID_Worker_3=argv[12];

   for(int i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

void create_table(sqlite3 *db)
{
   int ret;
   char sql[1024] = "\0";
   char *zErrMsg = 0;
   sprintf(sql, "create table if not exists COMPANY (id integer, support_plate text, beam text, motor text, lifting_columns text, accessory_package text, table_leg text, transmission text, instructions text);");
   ret = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK != ret){
		fprintf(stderr, "create table:%s\n", sqlite3_errmsg(db));
		fprintf(stderr, "create table:%s\n", zErrMsg);
		exit(EXIT_FAILURE);//异常退出
	}else{
      fprintf(stderr, "Create table successfully\n");
   }
}

int insert_item(sqlite3 *db, char **item_value)
{
   int ret;
   char sql[1024] = "\0";
   char *zErrMsg = 0;
   sprintf(sql, "insert into COMPANY (id, support_plate, beam, motor , lifting_columns, accessory_package, table_leg, transmission, instructions) values ('%s', '%s', '%s', '%s' , '%s', '%s', '%s', '%s', '%s');", item_value[0], item_value[1], item_value[2], item_value[3], item_value[4], item_value[5], item_value[6], item_value[7], item_value[8]);
   ret = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	if(SQLITE_OK != ret){
		fprintf(stderr, "insert into:%s\n", sqlite3_errmsg(db));
		fprintf(stderr, "insert into:%s\n", zErrMsg);
      return 0;//fail
	}else{
      fprintf(stderr, "Insert into table successfully\n");
      return 1;//succeed
   }
}

int find_item(sqlite3 *db, int id_value)
{
   int ret;
   char sql[1024] = "\0";
   char *zErrMsg = 0;
   const char* data = "Callback function called";
//   sprintf(sql, "Select * from COMPANY where id=%d;", id_value);
   sprintf(sql, "Select * from Test;");
   ret = sqlite3_exec(db, sql, date_callback, (void*)data, &zErrMsg);
	if(SQLITE_OK != ret){
        fprintf(stderr, "find item:%s\n", sqlite3_errmsg(db));
        fprintf(stderr, "find item:%s\n", zErrMsg);
      return 0;//fail
	}else{
      fprintf(stderr, "Insert into table successfully\n");
      return 1;//succeed
   }
}

void open_datebase(sqlite3 *db){
    int ret;
    ret = sqlite3_open("../database/et114.db", &db);
    if(SQLITE_OK != ret){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(EXIT_FAILURE);
    }else{
      fprintf(stderr, "Opened database successfully\n");
    }
}

int get_row_count(sqlite3 *db){
    int ret;
    char sql[1024] = "Select COUNT(*) from COMPANY;";
    char *zErrMsg = 0;
    ret = sqlite3_exec(db, sql, id_count_callback, NULL, &zErrMsg);
     if(SQLITE_OK != ret){
         fprintf(stderr, "insert into:%s\n", sqlite3_errmsg(db));
         fprintf(stderr, "insert into:%s\n", zErrMsg);
       return -1;//fail
     }else{
       fprintf(stderr, "Insert into table successfully\n");
       return id_num;//succeed
    }
}
