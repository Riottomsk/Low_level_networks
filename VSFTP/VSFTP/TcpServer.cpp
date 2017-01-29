#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "TcpServer.h"
#include <string>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <vector>




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

TcpServer::TcpServer(const char* port, std::string fname)
{
	filename = fname;
	std::cout << "Server created with port "<< port;
	DEFAULT_PORT = port;
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


int TcpServer::start_control()
{
	hListenThread = CreateThread(NULL, 0, &forCommandServerThread, this, 0, &dwListenThread);	
	return 0;
}

int TcpServer::start_recieve_data()
{
	std::cout << "Seerver started with " << filename;
	hListenThread = CreateThread(NULL, 0, &forDataServerThread, this, 0, &dwListenThread);	
	return 0;
}

DWORD TcpServer::forCommandServerThread(void *param)
{
	TcpServer *ts = (TcpServer*)param;
	ts->recieve_command();
	return 0;
}

std::string ws2ss(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
	return strTo;
}

void TcpServer::recieve_command()
{
	std::cout << "Start command reciever on port "<<DEFAULT_PORT<<"\n";
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
	std::cout << "Client accepted\n";
	// No longer need server socket
	closesocket(ListenSocket);

	std::vector<std::string> data_ports;
	std::vector<int> used_ports;

	for (char ch = '5'; ch <= '9'; ch++)
	{
		std::string base("2701");
		data_ports.push_back(base + ch);
		used_ports.push_back(0);
		//std::cout << (int)(ch - '5')<<" "<< data_ports[(int)(ch - '5')];
	}

	std::string command;	
	std::string arg;
	bool flag;



	std::string new_port = "";

	bool valid;
	// Receive until the peer shuts down the connection		
	do {

		valid = false;
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received on server: %d\n", iResult);

			
			command = "";
			arg = "";
			flag = true;
			for (int i = 0; i < iResult; i++)
			{
				if (recvbuf[i] == ' ')
					flag = false;
				else
				{
					if (flag)
						command += recvbuf[i];
					else
						arg += recvbuf[i];
				}
			}

			
			std::cout << recvbuf<<" "<<command<<"\n";


			if (command == "HELP")
			{
				valid = true;
				char *sendbuf = "LIST\nPASV\nRETR\nSTOR";
				iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				printf("Bytes Sent on server: %ld\n", iResult);

			}

			if (command == "PASV" && arg=="")
			{
				valid = true;
				std::string tmp("227 Entering Passive Mode(");
				for (int i = 0; i < data_ports.size(); i++)
				{
					if (!used_ports[i])
					{
						tmp += data_ports[i] + ')';
						new_port = data_ports[i];
						used_ports[i] = 1;
						break;
					}
					if (i == data_ports.size() - 1)
						tmp = "420 No ports availible";

				}

				char *sendbuf;
				sendbuf = new char[tmp.length()];
				strcpy(sendbuf, tmp.c_str());
				iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				printf("Bytes Sent on server: %ld\n", iResult);
			}

			if (command == "LIST" && arg == "")
			{
				valid = true;
				WIN32_FIND_DATA ffd;
				LARGE_INTEGER filesize;
				TCHAR szDir[MAX_PATH];
				HANDLE hFind = INVALID_HANDLE_VALUE;
				StringCchCopy(szDir, MAX_PATH, TEXT("server_files\\*"));
				_tprintf(TEXT("\nTarget directory is %s\n\n"), szDir);


				hFind = FindFirstFile(szDir, &ffd);
				std::string tmp;

				do
				{
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
					}
					else
					{
						filesize.LowPart = ffd.nFileSizeLow;
						filesize.HighPart = ffd.nFileSizeHigh;
						_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
						std::wstring filename(ffd.cFileName);
						tmp += ws2ss(filename)+"\n";
						
					}
				} while (FindNextFile(hFind, &ffd) != 0);

				std::cout << tmp << tmp.length()<<"\n"<<tmp[12]<<"\n";
				char *sendbuf;
				sendbuf = new char[tmp.length()];
				std::cout << (int)strlen(sendbuf);
				for (int i = 0; i < tmp.length(); i++)
				{
					sendbuf[i] = tmp[i];
				}
				//strcpy(sendbuf, tmp.c_str());
				//std::cout << (int)strlen(sendbuf);
				iResult = send(ClientSocket, sendbuf, (int)tmp.length(), 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				printf("Bytes Sent on server: %ld\n", iResult);
			}

			if (command == "RETR")
			{				
				valid = true;

				if (new_port == "")
				{
					char *sendbuf = "400 Use PASV first";
					iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ClientSocket);
						WSACleanup();
						return;
					}
					printf("Bytes Sent on server: %ld\n", iResult);
				}
				else
				{
					std::string filename("server_files\\"); //not from class
					filename += arg;
					FILE* test = fopen(filename.c_str(),"rb");
					if (!test)
					{
						char *sendbuf = "450 No Such file";
						iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ClientSocket);
							WSACleanup();
							return;
						}
						printf("Bytes Sent on server: %ld\n", iResult);

					}
					else
					{
						std::cout << "prepearing sender to send " << filename<<"\n";
						TcpClient sender(new_port.c_str(), filename.c_str());
						sender.connectToServer();

						char *sendbuf = "220 OK sending file";
						iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ClientSocket);
							WSACleanup();
							return;
						}
						printf("Bytes Sent on server: %ld\n", iResult);
					}
				}
							

			}

			if (command == "STOR")
			{
				valid = true;

			}

			if (!valid)
			{
				char *sendbuf = "Invalid command. Use HELP for commands";
				iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				printf("Bytes Sent on server: %ld\n", iResult);
			}
		}		

	} while (true);
	
	std::cout << "End of FTP command reciever\n";	
}


DWORD TcpServer::forDataServerThread(void *param)
{
	TcpServer *ts = (TcpServer*)param;
	std::cout << "Server was created with " << ts->filename;
	ts->recieve_data();
	return 0;
}

void TcpServer::recieve_data()
{
	std::cout << "Start data reciever\n";
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
	std::cout << "Try recieve to " << filename;
	FILE *output = std::fopen(filename.c_str(), "wb");
	char* name_for_output;
	bool name_sent = false;

	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);

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

	} while (iResult > 0 && keepAlive);

	fclose(output);
	std::cout << "End of reciever\n";
	//keepAlive = false;
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


void TcpServer::stop()
{
	keepAlive = false;
	closesocket(ClientSocket);
	WSACleanup();
	std::cout << "Server stopped";
}