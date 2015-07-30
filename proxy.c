/*
	compile using following line.
	gcc proxy.c libhiredis.a `mysql_config --cflags --libs`
*/


#include "proxy.h"

redisReply * redis_reply;
redisContext * redis_connection;

const char *redis_hostname = "127.0.0.1";
int redis_port = 6379;
char command[1024];
char **words;

MYSQL * mysql_connection;
MYSQL_RES *sql_result;
MYSQL_ROW sql_row;

const char *sql_host = "localhost";
const char *sql_user = "root";
const char *sql_user_pass = "hello";
const char *db_name = "redis_backup";
char query[1024];

int redis_status = DOWN;
int mysql_status= DOWN;


// Connects to redis and returns status as UP or DOWN 
int redis_connect(){
	redis_connection = redisConnect(redis_hostname,redis_port);
	//printf("%d\n",redis_connection->err);
	if(redis_connection->err){
		return DOWN;
	}
	return UP;
}

// Connects to mysql and returns status as UP or DOWN
int mysql_connect(){
	mysql_connection = mysql_init(NULL);
	if(mysql_connection == NULL)
		return DOWN;
	else if(mysql_real_connect(mysql_connection,sql_host,sql_user,
		sql_user_pass,db_name,0,NULL,0) == NULL)
		return DOWN;
	else
		return UP;
}

//reconnection using thread. 
void redis_reconnect(){
	do{
		redis_status = redis_connect();
	}while(redis_status==DOWN);
}

//reconnection using thread.
void mysql_reconnect(){
	do{
		mysql_status = mysql_connect();
	}while(mysql_status==DOWN);
}

// Initially starts redis and mysql
 
void proxy_init(){

	redis_status = redis_connect();
	mysql_status = mysql_connect();
}

// Slipts whole command and stores it in table of words
int split_command(){
	int i = 0;
	int j;
	words[i] = strtok(command," ");
	while(words[i] != NULL){
		//printf("%s\n",words[i]);
		i++;
		words[i] = strtok(NULL," ");
	}

	//Command to Uppercase
	j = 0;
	while(words[0][j]){
		words[0][j] = toupper(words[0][j]);
		j++;
	}
	return i;
}

// returns type of query as GET, SET or EXPIRE
int query_type(){
	
	//printf("In getQuery %s\n",words[0]);
	if(strcmp(words[0],"GET")==0)
		return GET;
	else if(strcmp(words[0],"SET")==0)
		return SET;
	else if(strcmp(words[0],"EXPIRE")==0)
		return EXPIRE;
	else 
		return 0;			
}

//fetches value from mysql  
char *get_from_mysql(){
	int i;
	time_t now;
	sprintf(query,"select redis_value,ttl from Map where redis_key =\'%s\'",words[1]);
	if(mysql_query(mysql_connection,query) == 0){	
		sql_result = mysql_store_result(mysql_connection);
		if(sql_result == NULL)
			return "nil";
		else{
			if((sql_row = mysql_fetch_row(sql_result))){
				if(atoll(sql_row[1]) == 0)
					return sql_row[0];
				else{
					now = time(NULL);
					if(atoll(sql_row[1]) > now )
						return sql_row[0];
					else 
						return "nil";
				}
			}else 
				return "nill";
					
		}
	}else{
		mysql_status = DOWN;
		// startup code
		return "NO Service";
	}
}	

//update to mysql for both SET and EXPIRE command. 
int set_to_mysql(int cmd){
	long long  ttl;
	time_t now;
	now = time(NULL);
	
	switch(cmd){
		case SET :
				sprintf(query,"insert into Map (redis_key,redis_value,ttl) values (\'%s\',\'%s\',%lld) on duplicate key update redis_value = \'%s\', ttl = %lld",words[1],words[2],0LL,words[2],0LL);
				break;
		case EXPIRE : 	ttl = atoll(words[2]);
				ttl = ttl + now;
				sprintf(query,"update Map set ttl = %lld where redis_key = \'%s\'",ttl,words[1]);
				break;
						
	}
	
	return mysql_query(mysql_connection,query);
}

int main(){
	
	int cmd_arg;
	pthread_t redis_th,mysql_th;
	proxy_init();
	printf("Redis Status %d\n",redis_status);
	printf("SQL Status %d\n",mysql_status);

	words = malloc(NO_ARG * sizeof(char *));
	
	do{
		// remove gets, its only for starter demos.
		gets(command);
		//printf("%s\n",command);	
	

		if(redis_status){
			//printf("%s\n",command);
			redis_reply = redisCommand(redis_connection,command);
			cmd_arg = split_command();
			if(redis_reply == NULL){
				redis_status = DOWN;
				pthread_create(&redis_th,NULL,&redis_reconnect,NULL);
				if(mysql_status){
					switch(query_type()){
						case GET    :
								printf("%s\n",get_from_mysql());
								break;
						case SET    :	
						case EXPIRE :	
			 					printf("Currently GET Queries are served.");
								break;	
					}
				}else{
					printf("NO SERVICE.\n");
					pthread_create(&mysql_th,NULL,&mysql_reconnect,NULL);
				}
			}else{
				printf("%s\n",redis_reply->str); 
				freeReplyObject(redis_reply);
				switch(query_type()){
				
	
					case GET    : 
							break;
					case SET    :	
							//Assumed sql will be up when redis is up.	
							set_to_mysql(SET);		
							break;
					case EXPIRE :	
							set_to_mysql(EXPIRE);
		 					break;	
				}
			}
		}else{
			pthread_create(&redis_th,NULL,&redis_reconnect,NULL);
			cmd_arg = split_command();
			if(query_type()==GET){
				if(mysql_status){
					printf("%s\n",get_from_mysql());
				}
				else{
					printf("NO SERVICES.");
					pthread_create(&mysql_th,NULL,&mysql_reconnect,NULL);	 
				}
				
			}else{
				printf("Currently Only GET Queries are served.");
			}
		}
	}while(strcmp(words[0],"QUIT"));	
	return 0;	
}




