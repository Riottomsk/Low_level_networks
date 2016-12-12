#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "TcpServer.h"

class TcpClient
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	//char *sendbuf = "this is a test";
	char sendbuf[DEFAULT_BUFLEN];
	//char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	//int recvbuflen = DEFAULT_BUFLEN;

	//bool keepAlive = true;
		
	HANDLE hListenThread;
	DWORD dwListenThread;

	static DWORD WINAPI forNewClientThread(void *param);
public:
	TcpClient();
	~TcpClient();

	int connectToServer();
	void sendSomeData();
	void stopClient();
};

