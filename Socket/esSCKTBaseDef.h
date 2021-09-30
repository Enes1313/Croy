#ifndef SCKT_BASEDEF_H
#define SCKT_BASEDEF_H

#include <stdlib.h>

#ifdef _WIN32

/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <windows.h>

#define EAScktType          SOCKET
#define SCKT_ERR            INVALID_SOCKET
#define VOIDCHAR            char
#define EAScktLen           int
#define RecvSendLenType     int
#define RecvSendRetType     int

#define SLEEP(ms)           do { Sleep(ms); } while(0)
#define CLEAR_TERMINAL()    system("cls")

#else

/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h> /* Needed for close, usleep */
#include <arpa/inet.h>
#include <term.h>
#include <termios.h>

#define EAScktType          int
#define SCKT_ERR            (-1)
#define VOIDCHAR            void
#define EAScktLen           socklen_t
#define RecvSendLenType     size_t
#define RecvSendRetType     ssize_t
#define SLEEP(ms)           do { usleep(ms * 1000); } while(0)
#define CLEAR_TERMINAL()    do { system("clear"); tigetstr( "clear" ); } while(0)

#endif

int eaSCKTInit(void);
void eaSCKTFinish(void);
void eaSCKTClose(EAScktType s);

#endif /* SCKT_BASEDEF_H  */
