#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "TcpClient.h"
#include <stdio.h>
#include <string>
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

TcpClient::TcpClient(const char* port, const char* fname)
{	
	if (fname)
	{
		filename = new char[strlen(fname)];
		strcpy(filename, fname);
	}
	DEFAULT_PORT = port;
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
	
	hListenThread = CreateThread(NULL, 0, &forNewClientThread, this, 0, &dwListenThread);
	
	return 0;
}

int TcpClient::connectToServerForCommand()
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

	//std::cout << "Creating client control thread";

	hListenThread = CreateThread(NULL, 0, &forCommandClientThread, this, 0, &dwListenThread);

	while(true)
	{ }	

	return 0;
}

void TcpClient::stopClient()
{
	std::cout << "Sending done\n";
	closesocket(ConnectSocket);
	WSACleanup();
	//keepAlive = false;
}


DWORD TcpClient::forNewClientThread(void *param)
{	
	TcpClient *tc = (TcpClient*)param;	
	tc->sendSomeData();
	return 0;
}

DWORD TcpClient::forCommandClientThread(void *param)
{
	TcpClient *tc = (TcpClient*)param;
	tc->sendSomeCommand();
	return 0;
}

void TcpClient::sendSomeCommand()
{
	std::cout << "Sending command on port " << DEFAULT_PORT << "\n";

	
	std::string command;	
	std::string new_port= "";
	TcpServer *reciever=nullptr;
	while (true)
	{
		
		std::getline(std::cin, command);
		
		char *sendbuf;
		sendbuf = new char[command.length()];
		strcpy(sendbuf, command.c_str()); 

			
		if (command.substr(0, 4) == "RETR")
		{
			std::string tmp("client_files\\");
			if (reciever)
			{
				//reciever->start_recieve_data();
				reciever->set_filename(tmp + command.substr(5));
			}
			//std::cout << "OK\n";

			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return;
			}
			printf("Bytes Sent on client: %ld\n", iResult);

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received on client: %d\n", iResult);
				std::cout << recvbuf << std::endl;
			}

			if (recvbuf[0] != 2)
			{
				if (reciever)
				{
					reciever->stop();
				}

				continue;
			}

		}

		// Send an initial buffer
		//std::cout << (int)strlen(sendbuf) << std::endl;
		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return;
		}
		printf("Bytes Sent on client: %ld\n", iResult);

		

		if (command == "PASV")
		{
			new_port = "";
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received on client: %d\n", iResult);
				std::cout << recvbuf << std::endl;
			}
			if (recvbuf[0] == '2')
			{
				for (int i = 0; i < iResult; i++)
				{
					if (recvbuf[i] == ')')
						break;

					new_port += recvbuf[i];

					if (recvbuf[i] == '(')
						new_port = "";
				}
				//std::cout << new_port;			

				reciever = new TcpServer(new_port.c_str());
				
				reciever->start_recieve_data();
			}
		}

		if (command.substr(0,4) == "STOR")
		{
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return;
			}
			printf("Bytes Sent on client: %ld\n", iResult);

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received on client: %d\n", iResult);
				//std::string tmp(recvbuf);
				//std::cout << recvbuf << tmp<<"test\ntest"<<std::endl;
				std::cout.write(&recvbuf[0], iResult) << "\n";
			}
			
		}

		if (command == "LIST")
		{
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received on client: %d\n", iResult);
				//std::string tmp(recvbuf);
				//std::cout << recvbuf << tmp<<"test\ntest"<<std::endl;
				std::cout.write(&recvbuf[0], iResult) << "\n";
			}
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

void TcpClient::sendSomeData()
{
	//filename = fname;
	std::cout << "Sending data\nOn port "<<DEFAULT_PORT<<"\n"<<"from "<< filename <<"wrf";
	
	//const char* full_file_name = "input\\WandP.txt"; // write here what you want to transport
	FILE *input =  std::fopen(filename, "rb");
		
	//std::cout << (int)strlen(full_file_name);
	
	
	if(ferror(input))
		std::cout << "File cant be opened!\n"; // сообщить об этом
	else
	{
		while (!feof(input))
		{
			//std::fgets(sendbuf, DEFAULT_BUFLEN, input);

			//char buffer[DEFAULT_BUFLEN];
			std::fread(&sendbuf[0], sizeof sendbuf[0], DEFAULT_BUFLEN, input);
			//int buf_size = (int)strlen(buffer);
		
		
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return;
			}
			//printf("Bytes Sent: %ld\n", iResult);
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