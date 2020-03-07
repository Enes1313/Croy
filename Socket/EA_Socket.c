#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "EA_Socket.h"

int Recver(int s, char * buf, unsigned char max_len)
{
	int de;
	unsigned char fe = 0, len;

	do{
		de = recv(s, (char *) &len, 1, 0);
	} while(!de);

	if(de == -1)
		return de;

	if (max_len < len)
	{
		return 0;
	}

	while(len > fe)
	{
		de = recv(s, buf + fe, len - fe, 0);

		if(de == -1)
			return de;
		fe = de + fe;
	}

	return fe;
}

int Sender(int s, const char * buf, unsigned char len)
{
	int de;
	unsigned char fe = 0;

	do{
		de = send(s, (char *) &len, 1, 0);
	} while(!de);

	if(de == -1)
		return de;

	while(len > fe)
	{
		de = send(s, buf + fe, len - fe, 0);

		if(de == -1)
			return de;
		fe = de + fe;
	}

	return fe;
}

char * recverText(int s)
{
	char * text;
	char buffer[3] = { 0 };
	unsigned short n = 0, total = 0, l = 0;

	if(Recver(s, buffer, 2) <= 0)
	{
		return NULL;
	}

	total = (unsigned short)((unsigned char)buffer[0]) + ((unsigned short)((unsigned char)buffer[1]) << 8);

	if ((text = (char *) calloc(total + 1, sizeof(char))) == NULL)
	{
		return NULL;
	}

	while (l != total)
	{
		if ((n = Recver(s, text + l, ((total - l) >= 0xFF) ? 0xFF : total - l)) <= 0)
		{
			break;
		}

		l += n;
	}

	return text;
}

void senderText(int s, char * text)
{
	char buffer[3] = { 0 };
	unsigned short n = 0, total = 0, l = 0;

	total = strlen(text);
	buffer[0] = total & 0xFF;
	buffer[1] = total >> 8;

	if(Sender(s, buffer, 2) <= 0)
	{
		return;
	}

	while (l != total)
	{
		if ((n = Sender(s, text + l, ((total - l) >= 0xFF) ? 0xFF : total - l)) <= 0)
		{
			break;
		}

		l += n;
	}
}

void senderFile(int s, char * path)
{
	FILE * file;
	int n, total = 0, l = 0;
	char sendbuf[0x100] = {0}, buffer[5] = { 0 };

	if((file = fopen(path, "rb")) == NULL)
	{
		senderText(s, "_Error_");
		return;
	}

	char *e = path + strlen(path) - 2;
	while ((e != (path - 1)) && (*e != '/' || *e != '\\'))
		e--;

	senderText(s, e + 1);

	fseek(file, 0L, SEEK_END);
	total = ftell(file);

	buffer[0] = total & 0xFF;
	buffer[1] = (total >> 8) & 0xFF;
	buffer[2] = (total >> 16) & 0xFF;
	buffer[3] = (total >> 24) & 0xFF;

	if(Sender(s, buffer, 4) <= 0)
	{
		return;
	}

	fseek(file, 0L, SEEK_SET);

	while ((n = fread(sendbuf, sizeof(char), ((total - l) >= 255) ? 255 : total - l, file)) > 0)
	{
		if (n != 255 && ferror(file))
		{
			break;
		}

		if (Sender(s, sendbuf, n) <= 0)
		{
			break;
		}
		l += n;
		memset(sendbuf, 0, 255);
	}

	fclose(file);
}

char * recverFile(int s)
{
	FILE * file;
	char * path;
	int n, total = 0;
	char sendbuf[256] = {0}, buffer[5] = { 0 };

	if ((path = recverText(s)) == NULL || strcmp("_Error_", path) == 0)
	{
		return NULL;
	}

	if((file = fopen(path, "wb")) == NULL)
	{
		return NULL;
	}

	if(Recver(s, buffer, 4) <= 0)
	{
		fclose(file);
		return NULL;
	}

	total = ((unsigned char)(buffer[0])) + (((unsigned char)(buffer[1])) << 8) + (((unsigned char)(buffer[2])) << 16) + (((unsigned char)(buffer[3])) << 24);

	while (total)
	{
		if ((n = Recver(s, sendbuf, 255)) <= 0)
		{
			fclose(file);
			return NULL;
		}

		n = fwrite(sendbuf, sizeof(char), n, file);

		if (n != 255 && ferror(file))
		{
			fclose(file);
			return NULL;
		}

		total -= n;
		memset(sendbuf, 0, 255);
	}

	fclose(file);

	return path;
}
