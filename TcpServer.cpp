#include "stdafx.h"
#include "TcpServer.h"


TcpServer::TcpServer()
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
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
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
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
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


TcpServer::~TcpServer()
{
}


int TcpServer::start()
{	
	TcpClient client;
	
	std::cout << "Creating reciever thread\n";

	hListenThread = CreateThread(NULL, 0, &forNewServerThread, this, 0, &dwListenThread);
	
	std::cout << "Make client connection\n";
	client.connectToServer();
	
	
	while (keepAlive)
	{ } //thats the main thread. it shouldnt finish until all work is done

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

DWORD TcpServer::forNewServerThread(void *param)
{
	TcpServer *ts = (TcpServer*)param;
	ts->recieve();
	return 0;
}

void TcpServer::recieve()
{
	std::cout << "Start reciever\n";
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// No longer need server socket
	closesocket(ListenSocket);


	// Receive until the peer shuts down the connection
	FILE *output = std::fopen("output\\output.txt", "w");
	//char* name_for_output;
	//bool name_sent = false;
	
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			/*
			TODO: actual name 
			if (!name_sent)
			{
				name_for_output = new char[iResult];
				output = std::fopen(name_for_output, "w");

			}
			*/

			std::fwrite(recvbuf, sizeof(char), iResult, output); // to proper use of sizeof should change revbuf to dynamic
			//std::cout << recvbuf<<"\n";
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);

	fclose(output);
	std::cout << "End of reciever\n";
	keepAlive = false;
}
