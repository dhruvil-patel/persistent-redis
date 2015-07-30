#ifndef __PROXY_H 
#define __PROXY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <my_global.h>
#include <mysql.h>
#include <pthread.h>
#include "hiredis.h"

#define NO_ARG 10
#define UP 1
#define DOWN 0

#define GET 1
#define SET 2
#define EXPIRE 3

extern int redis_status;
extern int mysql_status;

extern redisContext *redis_connection;
extern MYSQL *mysql_connection; 

int redis_connect();
int mysql_connect(); 






#endif
