// VSUdp.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "UdpServer.h"
#pragma comment (lib, "Ws2_32.lib")


int main()
{
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	UdpServer server;
	server.start();
    return 0;
}

