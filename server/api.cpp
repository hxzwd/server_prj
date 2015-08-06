#include "server.h"
#include "api.h"

/*
int f_reg_request(char *buf)
{
	char name[MAX_NAME_LEN];
	char *cur;
	int counter = 0;
	cur = strstr(buf, "?name=");
	if(cur != NULL)
	{

		while(buf[(int)(cur - buf) + counter + 6] != '&')
		{
			if(counter >= MAX_NAME_LEN - 1)
				return 0;
			name[counter] = buf[(int)(cur - buf) + counter + 6];
			counter++;
		}

		buf[counter] = '\0';
	
		strcpy(buf, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<p>");
		strcat(buf, name);
		strcat(buf, "<\p>");

	}
	else
	{
		return 0;
	}
	return 1;
}
*/