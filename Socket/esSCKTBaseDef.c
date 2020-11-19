#include "esSCKTBaseDef.h"

int eaSCKTInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 0), &wsa_data);
#else
    return 0;
#endif
}

int eaSCKTFinish(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

int eaSCKTClose(EAScktType sckt)
{
    int status = 0;

#ifdef _WIN32
    status = shutdown(sckt, SD_BOTH);
    if (status == 0) { status = closesocket(sckt); }
#else
    status = shutdown(sckt, SHUT_RDWR);
    if (status == 0) { status = close(sckt); }
#endif

    return status;
}