#include "esSCKTBaseDef.h"

int eaSCKTInit(void)
{
#ifdef _WIN32
    return WSAStartup(MAKEWORD(2, 2), &(WSADATA){0});
#else
    return 0;
#endif
}

void eaSCKTFinish(void)
{
#ifdef _WIN32
    (void)WSACleanup();
#endif
}

void eaSCKTClose(EAScktType s)
{
#ifdef _WIN32
    if (0 == shutdown(s, SD_BOTH))
    {
        (void)closesocket(s);
    }
#else
    if (0 == shutdown(s, SHUT_RDWR))
    {
        (void)close(s);
    }
#endif
}
