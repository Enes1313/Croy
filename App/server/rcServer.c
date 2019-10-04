#include <stdio.h>
#include <dirent.h>
#include <locale.h>
#include <windows.h>
#include "EA_Socket.h"

int port = 5005;

static void commWithSystem(void);
static int process(SOCKET sckt, char * al);
static void processCMD(SOCKET sckt);
static void processFileUpload(SOCKET sckt, char * path);
static void processFileDownload(SOCKET sckt, char * path);

int main()
{
	WSADATA _wsdata;

	setlocale(LC_ALL, "Turkish");

	while(1)
	{
		if (WSAStartup(MAKEWORD(2, 0), &_wsdata) == 0)
		{
			break;
		}

		Sleep(500);
	}

	commWithSystem();

	WSACleanup();

	return 0;
}

static void commWithSystem(void)
{
	int cnt, de;
	char al[1024] = {0};
	SOCKET sckt, istemci;
	struct sockaddr_in veriler, verileri;

	while (1)
	{
		sckt = socket(AF_INET, SOCK_STREAM, 0);

		veriler.sin_family = AF_INET;
		veriler.sin_port = htons(port);
		veriler.sin_addr.s_addr = INADDR_ANY;
		memset(&(veriler.sin_zero), 0, 8);

		bind(sckt, (struct  sockaddr *)&veriler, sizeof(struct  sockaddr));
		listen(sckt, 10);

		if (sckt != INVALID_SOCKET && sckt != SOCKET_ERROR)
		{
			while (1)
			{
				CNF_INFO("Baðlantý bekleniyor\n");
				cnt = sizeof(verileri);
				istemci = accept(sckt, (struct  sockaddr *)&verileri, &cnt);

				if(istemci != INVALID_SOCKET && istemci != SOCKET_ERROR)
				{
					break;
				}

				Sleep(500);
			}

			CNF_INFO("Ýstemci baðlandý\n");

			while(1)
			{
				CNF_INFO("Komut gir : ");

				gets(al);
				fflush(stdout);
				fflush(stdin);

				senderText(istemci, al);

				if ((de = process(istemci, al)) != 0)
				{
					CNF_INFO("Ýþlenemedi!\n");
					break;
				}
			}

			closesocket(sckt);
			sckt = INVALID_SOCKET;
			fflush(stdout);
		}

		Sleep(1000);
	}
}

int process(SOCKET sckt, char * al)
{
	if (strcmp(al, "cmd") == 0)
	{
		processCMD(sckt);
	}
	else if (strncmp(al, "getFile ", 8) == 0)
	{
		processFileDownload(sckt, al + 8);
	}
	else if (strncmp(al, "sendFile ", 9) == 0)
	{
		processFileUpload(sckt, al + 9);
	}
	else if (strcmp(al, "update") == 0)
	{
		// TODO: processUpdate(sckt);
	}

	return 0;
}

static void processCMD(SOCKET sckt)
{
	char * buffer;
	char input[300] = { 0 };

	while(1)
	{
		buffer = recverText(sckt);

		if (buffer == NULL)
		{
			CNF_INFO("çýkýþ 1\n");
			break;
		}

		if(strcmp(buffer, "_Error_") == 0)
		{
			CNF_INFO("çýkýþ 2\n");
			free(buffer);
			break;
		}

		printf(buffer);
		free(buffer);

		inp:
		gets(input);
		fflush(stdout);
		fflush(stdin);

		if (strlen(input) == 0)
		{
			goto inp;
		}

		senderText(sckt, input);
	}
}

static void processFileDownload(SOCKET sckt, char * path)
{
	char * fileName;

	fileName = recverFile(sckt);
	free(fileName);
}

static void processFileUpload(SOCKET sckt, char * path)
{
	senderFile(sckt, path);
}
/*
void processDir(SOCKET sckt, char * dir)
{
	char * fileName;

	fileName = recverFile(sckt);
	puts(fileName);
	free(fileName);
}*/
