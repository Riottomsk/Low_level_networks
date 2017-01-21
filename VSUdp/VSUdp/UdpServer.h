#pragma once
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#include "UdpClient.h"

class UdpServer
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	HANDLE hListenThread;
	DWORD dwListenThread;

	struct addrinfo *result = NULL;
	struct addrinfo hints;
	static DWORD WINAPI forNewServerThread(void *param);

	bool keepAlive = true;

public:
	UdpServer();
	~UdpServer();

	int start();
	void recieve();
	void stop();
};