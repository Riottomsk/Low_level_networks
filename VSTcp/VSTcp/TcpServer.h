#pragma once
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#include "TcpClient.h"

class TcpServer
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	HANDLE hListenThread;
	DWORD dwListenThread;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen =  DEFAULT_BUFLEN;

	static DWORD WINAPI forNewServerThread(void *param);

	bool keepAlive = true;

public:
	TcpServer();
	~TcpServer();

	int start();
	void recieve();
	void stop();
};

