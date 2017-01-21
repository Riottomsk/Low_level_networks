#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "UdpClient.h"


UdpClient::UdpClient()
{
	hListenThread = NULL;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &server_addr);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

}


UdpClient::~UdpClient()
{
}

int UdpClient::start()
{

	client_addr = *(sockaddr_in*)server_addr->ai_addr;
	client_addr.sin_port = 0;
	client_addr.sin_addr.S_un.S_addr = 0;
		
	ConnectSocket = socket(server_addr->ai_family, server_addr->ai_socktype,
		server_addr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		freeaddrinfo(server_addr);
		return 1;
	}

	// Setup socket
	iResult = bind(ConnectSocket, (sockaddr*)&client_addr, sizeof(client_addr));
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Bind failed" << std::endl;
		freeaddrinfo(server_addr);
		closesocket(ConnectSocket);
		return 1;
	}
	
	unsigned int max_dgram_size = 0;
	int uint_size = sizeof(max_dgram_size);
	iResult = getsockopt(ConnectSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&max_dgram_size, &uint_size);
	if (iResult == SOCKET_ERROR)
	{
		printf("getsockopt failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		closesocket(ConnectSocket);
		return 1;
	}
		

	std::cout << "Creating thread";	

	hListenThread = CreateThread(NULL, 0, &forNewClientThread, this, 0, &dwListenThread);
	return 0;
}

void UdpClient::stopClient()
{
	keepAlive = false;
	/*std::cout << "Sending done\n";
	closesocket(ConnectSocket);
	WSACleanup();
	*/
}


DWORD UdpClient::forNewClientThread(void *param)
{
	UdpClient *tc = (UdpClient*)param;
	tc->sendSomeData();
	return 0;
}

void UdpClient::sendSomeData()
{
	std::cout << "Sending data\n";

	const char* full_file_name = "input\\WandP.txt"; // write here what you want to transport options: WandP.txt CC.txt Hamlet.txt
	FILE *input = std::fopen(full_file_name, "rb");

	//std::cout << (int)strlen(full_file_name);


	if (ferror(input))
		std::cout << "File cant be opened!\n"; 
	else
	{
		/*
		TODO: actual file name, require parser

		//Send name of file
		send(ConnectSocket, full_file_name, (int)strlen(full_file_name), 0);
		if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
		}*/

		while (keepAlive && !feof(input))
		{
			//char *buffer = "Hello from client by UDP!\n";
			char buffer[DEFAULT_BUFLEN];
			std::fread(&buffer[0], sizeof buffer[0], DEFAULT_BUFLEN, input);
			int buf_size = (int)strlen(buffer);			
			iResult = sendto(ConnectSocket, buffer, DEFAULT_BUFLEN, 0, server_addr->ai_addr, (int)server_addr->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				std::cout << "Some error happend: " << WSAGetLastError()<<std::endl;				
				break;
			}
			Sleep(500);// or you can remove this to wait less
		}
		
	}

	keepAlive = false;

	freeaddrinfo(server_addr);
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	closesocket(ConnectSocket);
	std::cout << "End of sender\n";	

}