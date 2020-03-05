#include <stdio.h>
#include <windows.h>
#include <pthread.h>
#include "EA_Socket.h"

#define PORT 5005

void * threadHI(void * param);
static void commWithSystems(void);
static int process(int sckt, char * al);
static void recvSendText(int sckt);

int c = 1;
FD_SET sockets;
SOCKET selectedClient = -1;

int main()
{
	WSADATA _wsdata;

	if (WSAStartup(MAKEWORD(2, 0), &_wsdata) == 0)
	{
		commWithSystems();
	}
	else
	{
		puts("Error : WSAStartup()");
	}

	WSACleanup();
	getchar();
	return 0;
}

void * threadHI(void * param)
{
	int x;
	unsigned int var, i;
	char * al = (char *) param;
	struct sockaddr_in clientInfos;

	x = sizeof(struct  sockaddr);

	do {
		Sleep(1000);
		c = 1;
		selectedClient = -1;
		for (var = 0; var < sockets.fd_count; ++var)
		{
			getpeername(sockets.fd_array[var], (struct  sockaddr *)&clientInfos, &x);
			printf("Client Socket : %u, Client IP %s\n", (unsigned int) sockets.fd_array[var], inet_ntoa(clientInfos.sin_addr));
		}

		puts("Enter Socket\tConnect client");
		scanf("%u", &i);
		fflush(stdin);
		c = 0;
		system("cls");

		for (var = 0; var < sockets.fd_count; ++var)
		{
			if (sockets.fd_array[var] == i)
			{
				selectedClient = i;
				break;
			}
		}

		if (selectedClient != -1)
		{
			fgets(al, 300, stdin);
			al[strlen(al) - 1] = 0;
			senderText(selectedClient, al);

			if (process(selectedClient, al))
			{
				puts("eeeee");
				closesocket(selectedClient);
				FD_CLR(selectedClient, &sockets);
			}
			system("cls");
		}
	} while (1);

	return NULL;
}

static void commWithSystems(void)
{
	pthread_t th;
	char al[1024] = {0};
	int i, newClient, server;
	struct sockaddr_in myInfos, clientInfos;

	if (SOCKET_ERROR == (server = socket(AF_INET, SOCK_STREAM, 0)))
	{
		CNF_ERROR("socket");
		return;
	}

	myInfos.sin_family = AF_INET;
	myInfos.sin_port = htons(PORT);
	myInfos.sin_addr.s_addr = INADDR_ANY;
	memset(myInfos.sin_zero, 0, 8);

	if (SOCKET_ERROR == bind(server, (struct  sockaddr *)&myInfos, sizeof(struct  sockaddr)))
	{
		CNF_ERROR("bind");
		closesocket(server);
		return;
	}

	if (SOCKET_ERROR == listen(server, 10))
	{
		CNF_ERROR("listen");
		closesocket(server);
		return;
	}

	FD_ZERO(&sockets);
	FD_SET(server, &sockets);

	pthread_create(&th, NULL, threadHI, al);
	pthread_detach(th);

	for(;;)
	{
		if (SOCKET_ERROR == select(0, &sockets, NULL, NULL, NULL))
		{
			CNF_ERROR("select");
			break;
		}

		newClient = sockets.fd_array[0];

		if (newClient == server)
		{
			i = sizeof(clientInfos);
			if (SOCKET_ERROR != (newClient = accept(server, (struct  sockaddr *)&clientInfos, &i)))
			{
				if (sockets.fd_count == 64)
				{
					closesocket(newClient);
				}
				else
				{
					FD_SET(newClient, &sockets);
				}
			}
		}
		else
		{
			if ((selectedClient != newClient) || c)
			{
				FD_CLR(newClient, &sockets);
				closesocket(newClient);
			}
		}
	}

	closesocket(server);
}

int process(int sckt, char * al)
{
	if (strncmp(al, "cmd", 3) == 0)
	{
		recvSendText(sckt);
	}
	else if (strncmp(al, "getFile ", 8) == 0)
	{
		free(recverFile(sckt));
	}
	else if (strncmp(al, "sendFile ", 9) == 0)
	{
		senderFile(sckt, al + 9);
	}
	else if (strncmp(al, "update ", 7) == 0)
	{
		senderFile(sckt, al + 7);
		return 1;
	}

	return 0;
}

static void recvSendText(int sckt)
{
	char * buffer;
	char input[300 + 1] = { 0 };

	while(1)
	{
		buffer = recverText(sckt);

		if (buffer == NULL)
		{
			break;
		}

		if(strcmp(buffer, "_Error_") == 0)
		{
			free(buffer);
			break;
		}

		printf(buffer);
		free(buffer);

		fgets(input, 300, stdin);

		senderText(sckt, input);
	}
}
