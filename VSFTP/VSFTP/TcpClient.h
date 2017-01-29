#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "TcpServer.h"

class TcpClient
{
	const char* DEFAULT_PORT;// = "27015";
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	//char *sendbuf = "this is a test";
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	bool keepAlive = true;
		
	HANDLE hListenThread;
	DWORD dwListenThread;

	static DWORD WINAPI forNewClientThread(void *param);
	static DWORD WINAPI forCommandClientThread(void *param);

	//std::string filename;
	char* filename;
		
	//int * port_to_free;

public:
	TcpClient();
	TcpClient(const char* port, const char* fname = nullptr);
	~TcpClient();

	int connectToServer();
	int connectToServerForCommand();
	void sendSomeData();
	void sendSomeCommand();
	void stopClient();
};

