#include "esSCKTBaseDef.h"

int eaSCKTWSStart(void)
{
#ifdef _WIN32
    return WSAStartup(MAKEWORD(2, 2), &(WSADATA){0});
#else
    return 0;
#endif
}

void eaSCKTWSEnd(void)
{
#ifdef _WIN32
    (void)WSACleanup();
#endif
}

void eaSCKTClose(EASCKT s)
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

bool eaSCKTRecv(EASCKT s, char *buffer, unsigned char len)
{
    int de;
    unsigned char rlen = 0U;

    LOG("%s: len : %d\n", __func__, (int)len);

    while (len > rlen)
    {
        de = recv(s, &buffer[rlen], (int)(len - rlen), 0);

        if (0 >= de)
        {
            LOG("%s: error %d\n", __func__, de);

            return false;
        }

        rlen = (unsigned char)((unsigned int)de + rlen);
    }

    LOG("%s: finish\n", __func__);

    return true;
}

bool eaSCKTSend(EASCKT s, const char *buffer, unsigned char len)
{
    int de;
    unsigned char slen = 0U;

    LOG("%s: len : %d\n", __func__, (int)len);

    while (len > slen)
    {
        de = send(s, &buffer[slen], (int)(len - slen), 0);

        if (-1 == de)
        {
            LOG("%s: error\n", __func__);

            return false;
        }

        slen = (unsigned char)((unsigned int)de + slen);
    }

    LOG("%s: finish\n", __func__);

    return true;
}
