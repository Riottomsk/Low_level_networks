#include "stdafx.h"
#include "UdpServer.h"



UdpServer::UdpServer()
{
	hListenThread = NULL;
	// Initialize Winsock
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
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("1.socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the Udp listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);
}


UdpServer::~UdpServer()
{
}


int UdpServer::start()
{
	UdpClient client;

	std::cout << "Creating reciever thread\n";

	hListenThread = CreateThread(NULL, 0, &forNewServerThread, this, 0, &dwListenThread);

	std::cout << "Make client connection\n";
	client.start();

	char command;
	while (keepAlive)
	{
		std::cout << "Quit? - Q" << std::endl;
		std::cin >> command;
		if (command == 'Q')
			keepAlive = false;
			
	} //thats the main thread. it shouldnt finish until all work is done

	

	client.stopClient();
	// cleanup	
	WSACleanup();

	return 0;
}

DWORD UdpServer::forNewServerThread(void *param)
{
	UdpServer *ts = (UdpServer*)param;
	ts->recieve();
	return 0;
}

void UdpServer::recieve()
{
	std::cout << "Start reciever\n";

	unsigned int max_dgram_size = 0;
	int uint_size = sizeof(max_dgram_size);
	iResult = getsockopt(ListenSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&max_dgram_size, &uint_size);
	if (iResult == SOCKET_ERROR)
	{
		printf("getsockopt failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return;
	}
	fd_set readfds;
	char *dgram_buffer = (char*)malloc(max_dgram_size);

	FD_ZERO(&readfds);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	

	
	// Receive until the peer shuts down the connection
	FILE *output = std::fopen("output\\output.txt", "wb");
	//char* name_for_output;
	//bool name_sent = false;

	do {

		FD_SET(ListenSocket, &readfds);
		iResult = select(1, &readfds, NULL, NULL, &tv);
		if (iResult == -1)
		{
			std::cout<<"Error in select"<<std::endl;
		}
		else
			if (iResult == 0)
			{
				std::cout << "Timeout, no data after " << tv.tv_sec << " sec." << std::endl;
			}
			else
			{
				if (FD_ISSET(ListenSocket, &readfds))
				{
					sockaddr_in remote_addr;
					int addr_size = sizeof(remote_addr);
					int data = recvfrom(ListenSocket, (char*)dgram_buffer, max_dgram_size, 0, (sockaddr*)&remote_addr, &addr_size);
					if (data > 0)
					{						
						std::cout << data<< " bytes recieverd" << std::endl;
						std::fwrite(dgram_buffer, sizeof(char), data, output);
					}
				}
			}		

	} while (keepAlive);

	fclose(output);
	std::cout << "End of reciever\n";	
}
