#ifndef _INC_SCKT_BASEDEF
#define _INC_SCKT_BASEDEF

#include <stdlib.h>

#ifdef _WIN32
    /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501  /* Windows XP. */
    #endif
    #include <windows.h>
    #include <winsock2.h>

    #define EAScktType          SOCKET
    #define SCKT_ERR            SOCKET_ERROR
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

#ifdef SCKT_DEBUG_ENABLE
#include <stdio.h>
#define SCKT_DEBUG(...)         do {printf(__VA_ARGS__)} while(0)
#else
#define SCKT_DEBUG(...)         do { }while(0)
#endif /* SCKT_DEBUG_ENABLE  */

/*
    EAScktType socket(int, int, int);
    int setsockopt(EAScktType, int, int, const VOIDCHAR *, EAScktLen);
    int bind(EAScktType, const struct sockaddr *, EAScktLen);
    int listen(EAScktType, int);
    EAScktType accept(EAScktType, struct sockaddr *, EAScktLen *);
    int connect(EAScktType, const struct sockaddr *, EAScktLen);
    RecvSendRetType send(EAScktType, const VOIDCHAR *, RecvSendLenType, int);
    RecvSendRetType recv(EAScktType, VOIDCHAR *, RecvSendLenType, int);

    int select(int nfds, fd_set *, fd_set * , fd_set *, struct timeval *);

    int getpeername(EAScktType, struct sockaddr *, EAScktLen *);
    inet_ntoa, htons
*/

int eaSCKTInit(void);
int eaSCKTFinish(void);
int eaSCKTClose(EAScktType sock);

#endif /* _INC_SCKT_BASEDEF  */