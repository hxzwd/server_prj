#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <process.h>
#include <windows.h>

#include "sqlite3.h"

#define MESSAGE_TYPE_UNKNOWN 0
#define MESSAGE_TYPE_ERROR 1
#define MESSAGE_TYPE_GENERAL 2
#define MESSAGE_TYPE_CONNECTIONS 3


#define DEFAULT_DB_NAME "mdump.dblite"


#define TIME_LOG_BUFFER_SIZE 64

#define SERVER_DEFAULT_PORT 80
#define SERVER_INET_ADDR_PARAM "0.0.0.0"
#define SERVER_REQUEST_BUFFER_SIZE 64536
#define SERVER_MIDDLE_BUFFER_SIZE 64

void f_server_close(void);
int f_server_init(void);
void f_log(char *, int);
void f_parse_command_line(int, char **);
void f_parse_config_file(void);
void f_parse_request_buffer(char *);
unsigned _stdcall f_thread_function(void *);


//
//
//


int f_reg_request(char *);