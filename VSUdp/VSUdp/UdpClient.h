#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "UdpServer.h"

class UdpClient
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *server_addr = NULL,		
		hints;
	struct sockaddr_in client_addr;
	
	char sendbuf[DEFAULT_BUFLEN];
	
	int iResult;	

	bool keepAlive = true;

	HANDLE hListenThread;
	DWORD dwListenThread;

	static DWORD WINAPI forNewClientThread(void *param);
public:
	UdpClient();
	~UdpClient();

	int start();
	void sendSomeData();
	void stopClient();
};

