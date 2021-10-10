#ifndef SCKT_BASEDEF_H
#define SCKT_BASEDEF_H

#include <stdlib.h>
#include <stdbool.h>

#if OPEN_LOG == 1

#include <stdio.h>
#define LOG(f, ...) printf(f, ##__VA_ARGS__)

#else

#define LOG(f, ...)

#endif

#ifdef _WIN32

// stackoverflow.com/questions/12765743/getaddrinfo-on-win32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif

#include <winsock2.h>
#include <windows.h>

#define EASCKT          SOCKET
#define SCKT_ERR        INVALID_SOCKET
#define EAScktLen       int

#define SLEEP(ms)        Sleep(ms)
#define CLEAR_TERMINAL() system("cls")

#else

#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <term.h>
#include <termios.h>

#define EASCKT          int
#define SCKT_ERR        (-1)
#define EAScktLen       socklen_t

#define SLEEP(ms)        usleep(ms * 1000)
#define CLEAR_TERMINAL() do { system("clear"); tigetstr( "clear" ); } while(0)

#endif

/**
 * @brief 
 * 
 * @return int 
 */
int eaSCKTWSStart(void);

/**
 * @brief 
 * 
 */
void eaSCKTWSEnd(void);

/**
 * @brief 
 * 
 * @param s 
 */
void eaSCKTClose(EASCKT s);

/**
 * @brief 
 * 
 * @param s 
 * @param buffer 
 * @param len 
 * @return true 
 * @return false 
 */
bool eaSCKTRecv(EASCKT s, char *buffer, unsigned char len);

/**
 * @brief 
 * 
 * @param s 
 * @param buffer 
 * @param len 
 * @return true 
 * @return false 
 */
bool eaSCKTSend(EASCKT s, const char *buffer, unsigned char len);

#endif /* SCKT_BASEDEF_H  */
