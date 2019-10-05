#ifndef _INC_EA_SOCKET
#define _INC_EA_SOCKET

#define CNF_DEBUG	1

#include <windows.h>

#if CNF_DEBUG
#include <stdio.h>

#define CNF_ERROR(x)	perror(x)
#define CNF_INFO(...)	printf(__VA_ARGS__)
#else
#define CNF_EA_ERROR(x)
#define CNF_INFO(...)
#endif

int Recver(SOCKET, char *, unsigned char);
int Sender(SOCKET, const char *, unsigned char);
char * recverText(SOCKET);
void senderText(SOCKET, char *);
char * recverFile(SOCKET);
void senderFile(SOCKET, char *);

#endif
