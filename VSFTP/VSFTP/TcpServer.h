#pragma once
#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
#include "TcpClient.h"
#include <string>

class TcpServer
{
	//const int DEFAULT_BUFLEN = 512;
	const char* DEFAULT_PORT;// = "27015";

	//char * filename;

	std::string filename;

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
	static DWORD WINAPI forCommandServerThread(void *param);
	static DWORD WINAPI forDataServerThread(void *param);	

	bool keepAlive = true;

public:
	TcpServer();
	TcpServer(const char* port, std::string fname = "");
	~TcpServer();

	int start();
	int start_recieve_data();
	int start_control();
	void recieve();

	void set_filename(std::string fname) 
	{ 
		filename = fname;
		//std::cout << filename << " set\n"; 
	}

	void recieve_command();	
	void recieve_data();
	void stop();
};

