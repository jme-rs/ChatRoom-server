#ifndef DB_H
#define DB_H


#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "parson/parson.h"


JSON_Value *db_select_rooms(sqlite3 *db);
int         db_insert_rooms(sqlite3 *db, const char *room);
int         db_insert_msg(sqlite3    *db,
                          const char *id,
                          const char *room,
                          const char *msg);
JSON_Value *db_select_msg(sqlite3 *db, const char *room);


#endif // DB_H
