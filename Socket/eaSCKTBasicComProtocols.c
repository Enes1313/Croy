#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eaSCKTBasicComProtocols.h"

int recver(EAScktType s, char * bytes, unsigned char max_len)
{
	RecvSendRetType de;
	unsigned char fe = 0, len;

	do {
		de = recv(s, &len, 1, 0);
		if (de == -1)
			return de;
	} while(0);

	if (max_len < len)
		return 0;

	while (len > fe)
	{
		de = recv(s, bytes + fe, (RecvSendLenType) (len - fe), 0);

		if (de == -1)
			return de;
		fe = (unsigned char)de + fe;
	}

	return fe;
}

int sender(EAScktType s, const char * bytes, unsigned char len)
{
	RecvSendRetType de;
	unsigned char fe = 0;

	do {
		de = send(s, (char *) &len, 1, 0);
		if (de == -1)
			return de;
	} while(0);

	while(len > fe)
	{
		de = send(s, bytes + fe, (RecvSendLenType) (len - fe), 0);

		if(de == -1)
			return de;
		fe = (unsigned char)de + fe;
	}

	return fe;
}

char * recverText(EAScktType s)
{
	char * text;
	char buffer[3] = { 0 };
	int n = 0, total = 0, l = 0;

	if (recver(s, buffer, 2) <= 0)
		return NULL;

	total |= (buffer[0] | (buffer[1] << 8));

	if ((text = (char *) calloc(total + 1, sizeof(char))) == NULL)
		return NULL;

	do {
		if ((n = recver(s, text + l, ((total - l) >= 0xFF) ? 0xFF : (unsigned char)(total - l))) <= 0)
			break;
		l += n;
	} while (l != total);

	return text;
}

void senderText(EAScktType s, const char * text)
{
	char buffer[3] = { 0 };
	int n = 0, total = 0, l = 0;

	total = (int) strlen(text);
	buffer[0] = (char) (total & 0xFF);
	buffer[1] = (char) (total >> 8);

	if (sender(s, buffer, 2) <= 0)
		return;

	do {
		if ((n = sender(s, text + l, ((total - l) >= 0xFF) ? 0xFF : (unsigned char) (total - l))) <= 0)
			break;
		l += n;
	} while (l != total);
}

void senderFile(EAScktType s, const char * path)
{
	FILE * file;
	int n, total = 0, l = 0;
	char sendbuf[0x100] = {0}, buffer[5] = { 0 };

	if((file = fopen(path, "rb")) == NULL)
	{
		senderText(s, "_Error_");
		return;
	}

	const char * e = path + strlen(path) - 2;
	while ((e != (path - 1)) && (*e != '/' || *e != '\\'))
		e--;

	senderText(s, e + 1);

	fseek(file, 0, SEEK_END);
	total = (int) ftell(file);

	buffer[0] = (char) (total & 0xFF);
	buffer[1] = (char) ((total >> 8) & 0xFF);
	buffer[2] = (char) ((total >> 16) & 0xFF);
	buffer[3] = (char) ((total >> 24) & 0xFF);

	if(sender(s, buffer, 4) <= 0)
		return;

	fseek(file, 0, SEEK_SET);

	while ((n = (int) fread(sendbuf, sizeof(char), ((total - l) >= 255) ? 255 : (unsigned char) (total - l), file)) > 0)
	{
		if (n != 255 && ferror(file))
			break;

		if (sender(s, sendbuf, (unsigned char) n) <= 0)
			break;
		l += n;
		memset(sendbuf, 0, 255);
	}

	fclose(file);
}

char * recverFile(EAScktType s)
{
	FILE * file;
	char * path;
	int n, total = 0;
	char sendbuf[256] = {0}, buffer[5] = { 0 };

	if ((path = recverText(s)) == NULL || strcmp("_Error_", path) == 0)
		return NULL;

	if((file = fopen(path, "wb")) == NULL)
		return NULL;

	if(recver(s, buffer, 4) <= 0)
	{
		fclose(file);
		return NULL;
	}

	total |= (buffer[0] | (buffer[1] << 8) | (buffer[2] << 15) | (buffer[3] << 24));

	while (total)
	{
		if ((n = recver(s, sendbuf, 255)) <= 0)
		{
			fclose(file);
			return NULL;
		}

		n = (int) fwrite(sendbuf, sizeof(char), n, file);

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