#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eaSCKTBasicComProtocols.h"

static bool recver(EAScktType s, char *buffer, unsigned char len)
{
    RecvSendRetType de;
    unsigned char rlen = 0U;

    do 
    {
        unsigned char inBeingLen;

        de = recv(s, (char *)&inBeingLen, 1, 0);

        if (-1 == de)
        {
            return false;
        }

        if ((de > 0) && (len != inBeingLen))
        {
            return false;
        }
    } while (0 == de);

    while (len > rlen)
    {
        de = recv(s, &buffer[rlen], (RecvSendLenType)(len - rlen), 0);

        if (-1 == de)
        {
            return false;
        }

        rlen = (unsigned char)((unsigned int)de + rlen);
    }

    return true;
}

static bool sender(EAScktType s, const char *buffer, unsigned char len)
{
    RecvSendRetType de;
    unsigned char slen = 0U;

    do 
    {
        de = send(s, (char *)&len, 1, 0);
        
        if (-1 == de)
        {
            return false;
        }
    } while (0 == de);

    while (len > slen)
    {
        de = send(s, &buffer[slen], (RecvSendLenType)(len - slen), 0);

        if (-1 == de)
        {
            return false;
        }

        slen = (unsigned char)((unsigned int)de + slen);
    }

    return true;
}

bool recverText(EAScktType s, char *text, unsigned int size)
{
    unsigned char buffer[2];
    
    if (false == recver(s, (char *)buffer, 2U))
    {
        return false;
    }

    unsigned int total = ((unsigned int)buffer[0] | 
                          (unsigned int)(buffer[1] << 8));

    if (size < (total + 1U))
    {
        return false;
    }

    text[total] = '\0';

    unsigned int sended = 0U;

    do 
    {
        unsigned char len = ((total - sended) >= 0xFFU) ? 
                            (unsigned char)0xFFU : 
                            (unsigned char)(total - sended);

        if (false == recver(s, &text[sended], len))
        {
            break;
        }

        sended += len;
    } while (sended < total);

    if (sended < total)
    {
        return false;
    }

    return true;
}

bool senderText(EAScktType s, const char *text)
{
    unsigned int total = (unsigned int) strlen(text);

    unsigned char buffer[2] = {
        (unsigned char)(total & 0xFFU),
        (unsigned char)(total >> 8)
    };
    
    if (false == sender(s, (char *)buffer, 2U))
    {
        return false;
    }

    unsigned int sended = 0U;

    do 
    {
        unsigned char len = ((total - sended) >= 0xFFU) ? 
                            (unsigned char)0xFFU : 
                            (unsigned char)(total - sended);

        if (false == sender(s, &text[sended], len))
        {
            break;
        }

        sended += len;
    } while (sended < total);

    if (sended < total)
    {
        return false;
    }

    return true;
}

bool recverFile(EAScktType s)
{
    char name[260 + 1];
    
    if ((false == recverText(s, name, sizeof(name))) || 
        !strcmp("_Error_", name))
    {
        return false;
    }

    unsigned char buffer[4];

    if (false == recver(s, (char *)buffer, 4U))
    {
        return false;
    }

    FILE *file = fopen(name, "wb");

    if (NULL == file)
    {
        return false;
    }

    char sendbuf[255];
    unsigned int sended = 0;
    unsigned int total = (unsigned int)buffer[0] | 
                         ((unsigned int)buffer[1] << 8) | 
                         ((unsigned int)buffer[2] << 16) | 
                         ((unsigned int)buffer[3] << 24);

    do 
    {
        unsigned char len = ((total - sended) >= 0xFFU) ? 
                            (unsigned char)0xFFU : 
                            (unsigned char)(total - sended);

        if (false == recver(s, sendbuf, len))
        {
            (void)fclose(file);
            (void)remove(name);

            return false;
        }

        (void)fwrite(sendbuf, sizeof(char), len, file);

        sended += len;
    } while (sended < total);

    (void)fclose(file);

    if (sended < total)
    {
        (void)remove(name);

        return false;
    }

    return true;
}

bool senderFile(EAScktType s, const char *path)
{
    const char *name = path;

    for (size_t idx = strlen(path); 0U != idx; idx--)
    {
        if ((path[idx - 1U] == '/') || (path[idx - 1U] == '\\'))
        {
            name = &path[idx];

            break;
        }
    }

    FILE *file = fopen(path, "rb");

    if (NULL == file)
    {
        senderText(s, "_Error_");

        return false;
    }
    
    (void)fseek(file, 0, SEEK_END);

    unsigned int total = (unsigned int)ftell(file);

    (void)fseek(file, 0, SEEK_SET);

    unsigned char buffer[4] = {
        (unsigned char)(total & 0xFFU),
        (unsigned char)((total >> 8) & 0xFFU),
        (unsigned char)((total >> 16) & 0xFFU),
        (unsigned char)((total >> 24) & 0xFFU)
    };

    if (false == senderText(s, name))
    {
        (void)fclose(file);

        return false;
    }

    if (false == sender(s, (char *)buffer, 4U))
    {
        (void)fclose(file);

        return false;
    }

    char sendbuf[255];
    unsigned int sended = 0;

    do 
    {
        unsigned char len = ((total - sended) >= 0xFFU) ? 
                            (unsigned char)0xFFU : 
                            (unsigned char)(total - sended);

        (void)fread(sendbuf, sizeof(char), len, file);

        if (false == sender(s, sendbuf, (unsigned char) len))
        {
            break;
        }

        sended += len;
    } while (sended < total);

    (void)fclose(file);

    if (sended < total)
    {
        return false;
    }

    return true;
}
