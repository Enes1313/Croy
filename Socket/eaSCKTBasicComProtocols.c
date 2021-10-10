#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eaSCKTBasicComProtocols.h"

static bool recver(EASCKT s, char *buffer, unsigned char len);
static bool sender(EASCKT s, const char *buffer, unsigned char len);

bool recverText(EASCKT s, char *text, unsigned int size)
{
    unsigned char buffer[2];

    LOG("\t\t%s: size : %d\n", __func__, size);
    
    if (false == recver(s, (char *)buffer, 2U))
    {
        LOG("\t\t%s: error get total len\n", __func__);

        return false;
    }

    unsigned int total = (unsigned int)buffer[0] | 
                         (unsigned int)(buffer[1] << 8);

    if (size < (total + 1U))
    {
        LOG("\t\t%s: error total len not equal size\n", __func__);

        return false;
    }

    text[total] = '\0';

    unsigned int recved = 0U;

    do 
    {
        unsigned char len = ((total - recved) >= 0xFFU) ? 
                            (unsigned char)0xFFU : 
                            (unsigned char)(total - recved);

        if (false == recver(s, &text[recved], len))
        {
            LOG("\t\t%s: error get text\n", __func__);

            break;
        }

        recved += len;
    } while (recved < total);

    if (recved < total)
    {
        LOG("\t\t%s: error recved < total\n", __func__);

        return false;
    }

    LOG("\t\t%s: finish\n", __func__);

    return true;
}

bool senderText(EASCKT s, const char *text)
{
    unsigned int total = (unsigned int) strlen(text);

    unsigned char buffer[2] = {
        (unsigned char)(total & 0xFFU),
        (unsigned char)(total >> 8)
    };

    LOG("\t\t%s: total : %d\n", __func__, total);
    
    if (false == sender(s, (char *)buffer, 2U))
    {
        LOG("\t\t%s: error send total len\n", __func__);

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
            LOG("\t\t%s: error send text\n", __func__);

            break;
        }

        sended += len;
    } while (sended < total);

    if (sended < total)
    {
        LOG("\t\t%s: error sended < total\n", __func__);

        return false;
    }

    LOG("\t\t%s: finish\n", __func__);

    return true;
}

bool recverFile(EASCKT s)
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

bool senderFile(EASCKT s, const char *path)
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

static bool recver(EASCKT s, char *buffer, unsigned char len)
{
    unsigned char inBeingLen;

    LOG("\t%s: len : %d\n", __func__, len);

    if (false == eaSCKTRecv(s, (char *)&inBeingLen, 1U))
    {
        LOG("\t%s: error recv inBeingLen\n", __func__);

        return false;
    }

    if (len != inBeingLen)
    {
        LOG("\t%s: error len != inBeingLen\n", __func__);

        return false;
    }

    if (false == eaSCKTRecv(s, buffer, len))
    {
        LOG("\t%s: error recv buffer\n", __func__);

        return false;
    }

    LOG("\t%s: finish\n", __func__);

    return true;
}

static bool sender(EASCKT s, const char *buffer, unsigned char len)
{
    LOG("\t%s: len : %d\n", __func__, len);

    if (false == eaSCKTSend(s, (char *)&len, 1U))
    {
        LOG("\t%s: error send len\n", __func__);

        return false;
    }

    if (false == eaSCKTSend(s, buffer, len))
    {
        LOG("\t%s: error send buffer\n", __func__);

        return false;
    }
    
    LOG("\t%s: finish\n", __func__);

    return true;
}
