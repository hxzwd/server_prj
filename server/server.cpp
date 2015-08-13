#include "server.h"
#include "api.h"

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "WSock32.lib")
#pragma comment(lib, "sqlite3.lib")

FILE *log_error, *log_general, *log_connect;

struct sockaddr_in server_addr;

SOCKET server_socket;

int server_port = SERVER_DEFAULT_PORT;
int opt_stats = 1;

sqlite3 *server_db;

int f_server_init()
{
	
	if((log_error = fopen("error.log", "a+")) == NULL)
		return 0;

	if((log_general = fopen("stats.log", "a+")) == NULL)
		return 0;

	if((log_connect = fopen("connects.log", "a+")) == NULL)
		return 0;
	
	f_log("session started\n", MESSAGE_TYPE_GENERAL);
	
	return 1;
}

void f_log(char *message, int type)
{
	
	FILE *destination;
	time_t time_now;
	struct tm *time_now_u;
	char buffer[TIME_LOG_BUFFER_SIZE];
	
	switch(type)
	{
		case MESSAGE_TYPE_ERROR:
			destination = log_error;
			break;
		case MESSAGE_TYPE_GENERAL:
			destination = log_general;
			break;
		case MESSAGE_TYPE_CONNECTIONS:
			destination = log_connect;
			break;
		default:
			destination = stderr;
	}

	time_now = time(NULL);
	time_now_u = localtime(&time_now);

	if(!strftime(buffer, TIME_LOG_BUFFER_SIZE, "%d.%m.%Y %H:%M:%S ", time_now_u))
	{
		fprintf(destination, "%s", "bad_time ");
	}
	else
	{
		fprintf(destination, "%s", buffer);
	}

	fprintf(destination, "%s", message);


}

void f_close_server(void)
{
	f_log("session closed\n", MESSAGE_TYPE_GENERAL);
	if(WSACleanup())
	{
		f_log("wsa cleanup error\n", MESSAGE_TYPE_ERROR);
	}

	if(server_socket >= 0)
	{
		closesocket(server_socket);
	}

	fclose(log_general);
	fclose(log_error);
}

void f_parse_command_line(int argc, char *argv[])
{
	server_port = SERVER_DEFAULT_PORT;
	opt_stats = 1;
}

void f_parse_config_file(char *cfg_file)
{
	FILE *cfg;
	if((cfg = fopen(cfg_file, "r")) == NULL)
	{
		printf("cannot open %s\n", cfg_file);
		return;
	}

	char cmd[MAX_CMD_SIZE];

	//fgets(cmd, MAX_CMD_SIZE, cfg);
	//printf("CMD: %s", cmd);
	
	
	fclose(cfg);
}



int f_reg_request(char *buf)
{
	char name[MAX_NAME_LEN + 1];
	char pass[MAX_PASS_LEN + 1];
	char email[MAX_EMAIL_LEN + 1];
	char *cur;
	int counter = 0;
	cur = strstr(buf, "?name=");
	if(cur != NULL)
	{

		while(buf[(int)(cur - buf) + counter + 6] != '&')
		{
			if(counter >= MAX_NAME_LEN)
				return 0;
			name[counter] = buf[(int)(cur - buf) + counter + 6];
			counter++;
		}

		name[counter] = '\0';
	
		cur = strstr(buf, "&pass=");

		if(cur != NULL)
		{
			counter = 0;
			while(buf[(int)(cur - buf) + counter + 6] != '&')
			{
				if(counter >= MAX_PASS_LEN)
					return 0;
				pass[counter] = buf[(int)(cur - buf) + counter + 6];
				counter++;
			}

		pass[counter] = '\0';
		cur = strstr(buf, "&email=");
		
		}
		else
			return 0;

		if(cur != NULL)
		{
			counter = 0;
			while(buf[(int)(cur - buf) + counter + 7] != ' ')
			{
				if(counter >= MAX_EMAIL_LEN)
					return 0;
				email[counter] = buf[(int)(cur - buf) + counter + 7];
				counter++;
			}
		}
		else
			return 0;

		email[counter] = '\0';
	}
	else
	{
		return 0;
	}


	char *sqlite_cmd = NULL;
	sqlite_cmd = (char *)malloc(strlen("INSERT INTO USERS_T (NAME, EMAIL, PASS) VALUES('', '', '')") + strlen(name) + strlen(pass) + strlen(email) + 1);
	
	if(sqlite_cmd == NULL)
	{
		return 0;
	}

	strcpy(sqlite_cmd, "INSERT INTO USERS_T (NAME, EMAIL, PASS) VALUES('");
	strcat(sqlite_cmd, name);
	strcat(sqlite_cmd, "', '");
	strcat(sqlite_cmd, pass);
	strcat(sqlite_cmd, "', '");
	strcat(sqlite_cmd, email);
	strcat(sqlite_cmd, "')");

	HANDLE hMutex = (HANDLE)server_db;
	
	WaitForSingleObject(hMutex, INFINITE);
	
	if(sqlite3_exec(server_db, sqlite_cmd, NULL, NULL, NULL))
	{

		strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>");
		strcat(buf, name);
		strcat(buf, "|");
		strcat(buf, pass);
		strcat(buf, "|");
		strcat(buf, email);
		strcat(buf, " - reg error<\p>");

	}
	else
	{
	
		strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>reg ok<\p>");
	
	}
	
	ReleaseMutex(hMutex);

	free(sqlite_cmd);

	return 1;
}



void f_parse_request_buffer(char *buf)
{

	int counter = 0;
	char accum[SERVER_MIDDLE_BUFFER_SIZE] = { 0 };
	memcpy(accum, buf, 9); // GET /api/|9 symbols

	if(!strcmp(accum, "GET /api/"))
	{
		while(buf[counter + 9] != '/') counter++;
		memcpy(accum, buf + 9, counter + 1);
		accum[counter + 1] = '\0';

		if(!strcmp(accum, "reg/"))
		{
			if(f_reg_request(buf))
			{
				
			}
			else
			{
				strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>API REG REQUEST(0)</p>");
			}
		}
		else if(!strcmp(accum, "func0/"))
		{
			strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>API func0 REQUEST</p>");
		}
		else if(!strcmp(accum, "func1/"))
		{
			strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>API func1 REQUEST</p>");
		}
		else
		{
			strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>API (UNKNOWN) REQUEST</p>");
		}

	}
	else
	{
		strcpy(buf, "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<p>sorry, 404 error</p>");
	}

}

unsigned _stdcall f_thread_function(void *client)
{
	if(opt_stats)
		printf("[%d]client connected\n", *(SOCKET *)client);
		
	char *buffer = (char *)malloc(SERVER_REQUEST_BUFFER_SIZE);

	if(buffer != NULL)
	{
		recv(*(SOCKET *)client, buffer, SERVER_REQUEST_BUFFER_SIZE, 0);

		f_parse_request_buffer(buffer);
		
		send(*(SOCKET *)client, buffer, strlen(buffer), 0);
		free(buffer);
	}

	if(opt_stats)
		printf("[%d]client disconnected\n", *(SOCKET *)client);

	closesocket(*(SOCKET *)client);
	free(client);
	return 1;
}

int main(int argc, char *argv[])
{
	char buffer[1024];
	
	f_parse_config_file("default.cfg");
	f_parse_command_line(argc, argv);

	if(!f_server_init())
	{
		printf("server init error");
		return 1;
	}

	if(WSAStartup(0x202, (WSADATA *)buffer))
	{
		f_log("wsa startup error\n", MESSAGE_TYPE_ERROR);
		return 1;
	}
	
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{

		f_log("socket error\n", MESSAGE_TYPE_ERROR);
		f_close_server();
		exit(1);
	}
	
	if(sqlite3_open(DEFAULT_DB_NAME, &server_db))
	{
		f_log("sqlite open error\n", MESSAGE_TYPE_ERROR);
		f_close_server();
		sqlite3_close(server_db);
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_INET_ADDR_PARAM);

	if((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
	{
		f_log("bind error\n", MESSAGE_TYPE_ERROR);
		f_close_server();
		exit(1);
	}

	listen(server_socket, 0);

	SOCKET *cl;

	if(sqlite3_exec(server_db, "CREATE TABLE IF NOT EXISTS USERS_T(ID BIGINT UNSIGNED AUTO_INCREMENT, NAME TEXT, EMAIL TEXT, PASS TEXT)", NULL, NULL, NULL))
	{
		f_log("sqlite create table users error\n", MESSAGE_TYPE_ERROR);
		f_close_server();
		sqlite3_close(server_db);
		exit(1);
	}

	if(sqlite3_exec(server_db, "CREATE TABLE IF NOT EXISTS DATA_T(ID BIGINT UNSIGNED, ARTICLE TEXT, DT DATE, TM TIME)", NULL, NULL, NULL))
	{
		f_log("sqlite create table data error\n", MESSAGE_TYPE_ERROR);
		f_close_server();
		sqlite3_close(server_db);
		exit(1);
	}


	struct sockaddr_in client_info;
	
	std::string addr;

	while(1)
	{
		cl = (SOCKET *)malloc(sizeof(SOCKET));
		*cl = accept(server_socket, (sockaddr *)&client_info, NULL);

		addr = inet_ntoa(client_info.sin_addr) + '\n';
		f_log((char *)addr.c_str(), MESSAGE_TYPE_CONNECTIONS);

/*
		f_log(inet_ntoa(client_info.sin_addr), MESSAGE_TYPE_CONNECTIONS);
*/
		_beginthreadex(NULL, 0, &f_thread_function, (void *)cl, 0, NULL);
	
	}

	f_close_server();
	sqlite3_close(server_db);

	return 0;
}