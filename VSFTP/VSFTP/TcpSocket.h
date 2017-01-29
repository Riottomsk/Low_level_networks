#pragma once
#define DEFAULT_BUFLEN 512
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

class TcpSocket
{
	const char* DEFAULT_PORT;

	//std::string filename;

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	HANDLE hListenThread;
	DWORD dwListenThread;

	//struct addrinfo *result = NULL;
	//struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	
	static DWORD WINAPI forCommandServerThread(void *param);
	static DWORD WINAPI forDataServerThread(void *param);

	bool keepAlive = true;

	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	//char *sendbuf = "this is a test";
	char sendbuf[DEFAULT_BUFLEN];
	//char recvbuf[DEFAULT_BUFLEN];
	//int iResult;
	//int recvbuflen = DEFAULT_BUFLEN;

	//bool keepAlive = true;

	//HANDLE hListenThread;
	//DWORD dwListenThread;

	static DWORD WINAPI forNewClientThread(void *param);
	static DWORD WINAPI forCommandClientThread(void *param);

	//std::string filename;
	char* filename;

	//int * port_to_free;

public:	
	TcpSocket(const char* port, const char* fname = "");
	~TcpSocket();

	
	int start_recieve_data();
	int start_recieve_control();
	
	void set_filename(const char* fname)
	{
		filename = new char[strlen(fname)];
		strcpy(filename, fname);
	}

	void recieve_command();
	void recieve_data();

	int connectToServer();
	int connectToServerForCommand();
	void sendSomeData();
	void sendSomeCommand();

	void stop();
};

