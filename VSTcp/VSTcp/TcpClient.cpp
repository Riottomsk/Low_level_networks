#include "stdafx.h"
#include "TcpClient.h"


TcpClient::TcpClient()
{
	hListenThread = NULL;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

}


TcpClient::~TcpClient()
{
}

int TcpClient::connectToServer()
{
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	std::cout << "Creating thread";
	/*
	std::thread t(this->sendSomeData);

	if (t.joinable())
		t.join();
	*/

	hListenThread = CreateThread(NULL, 0, &forNewClientThread, this, 0, &dwListenThread);

	

	/*
	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);
	*/
	// cleanup
	

	return 0;
}

void TcpClient::stopClient()
{
	std::cout << "Sending done\n";
	closesocket(ConnectSocket);
	WSACleanup();
}


DWORD TcpClient::forNewClientThread(void *param)
{
	TcpClient *tc = (TcpClient*)param;
	tc->sendSomeData();
	return 0;
}

void TcpClient::sendSomeData()
{
	std::cout << "Sending data\n";
	
	const char* full_file_name = "input\\WandP.txt"; // write here what you want to transport
	FILE *input =  std::fopen(full_file_name, "r");
		
	//std::cout << (int)strlen(full_file_name);
	
	
	if(ferror(input))
		std::cout << "File cant be opened!\n"; // сообщить об этом
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

		while (!feof(input))
		{
			std::fgets(sendbuf, DEFAULT_BUFLEN, input);

		// Send an initial buffer
			//std::cout << sendbuf;
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return;
			}
			printf("Bytes Sent: %ld\n", iResult);
		}
	}
		

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	std::cout << "End of sender\n";

	stopClient();

}