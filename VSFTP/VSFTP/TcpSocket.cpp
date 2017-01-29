#include "stdafx.h"
#include "TcpSocket.h"


std::vector<std::string> data_ports;
std::vector<int> used_ports;



TcpSocket::TcpSocket(const char* port, const char* fname)
{
	if (fname)
	{
		filename = new char[strlen(fname)];
		strcpy(filename, fname);
	}
	DEFAULT_PORT = port;
	hListenThread = NULL;
}


TcpSocket::~TcpSocket()
{
}




int TcpSocket::start_recieve_control()
{
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
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
		return 0;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	freeaddrinfo(result);
	hListenThread = CreateThread(NULL, 0, &forCommandServerThread, this, 0, &dwListenThread);
	return 0;
}

int TcpSocket::start_recieve_data()
{	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
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
		return 0;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 0;
	}

	freeaddrinfo(result);
	hListenThread = CreateThread(NULL, 0, &forDataServerThread, this, 0, &dwListenThread);
	return 0;
}

DWORD TcpSocket::forCommandServerThread(void *param)
{
	TcpSocket *ts = (TcpSocket*)param;
	ts->recieve_command();
	return 0;
}

std::string ws2s(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
	return strTo;
}

void TcpSocket::recieve_command()
{
	std::cout << "Start command reciever on port " << DEFAULT_PORT << "\n";
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
			//printf("Bytes received on server: %d\n", iResult);


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


			//std::cout << recvbuf << " " << command << "\n";


			if (command == "HELP")
			{
				valid = true;
				char *sendbuf = "LIST\nPASV\nRETR\nSTOR\nQUIT";
				iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				//printf("Bytes Sent on server: %ld\n", iResult);

			}

			if (command == "PASV" && arg == "")
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
				//printf("Bytes Sent on server: %ld\n", iResult);
			}

			if (command == "LIST" && arg == "")
			{
				valid = true;
				WIN32_FIND_DATA ffd;
				LARGE_INTEGER filesize;
				TCHAR szDir[MAX_PATH];
				HANDLE hFind = INVALID_HANDLE_VALUE;
				StringCchCopy(szDir, MAX_PATH, TEXT("server_files\\*"));
				//_tprintf(TEXT("\nTarget directory is %s\n\n"), szDir);


				hFind = FindFirstFile(szDir, &ffd);
				std::string tmp;

				do
				{
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
					}
					else
					{
						filesize.LowPart = ffd.nFileSizeLow;
						filesize.HighPart = ffd.nFileSizeHigh;
						//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
						std::wstring filename(ffd.cFileName);
						tmp += ws2s(filename) + "\n";

					}
				} while (FindNextFile(hFind, &ffd) != 0);

				//std::cout << tmp << tmp.length() << "\n" << tmp[12] << "\n";
				char *sendbuf;
				sendbuf = new char[tmp.length()];
				//std::cout << (int)strlen(sendbuf);
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
				//printf("Bytes Sent on server: %ld\n", iResult);
			}
			if (command == "QUIT")
			{
				valid = true;
				keepAlive = false;
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
					//printf("Bytes Sent on server: %ld\n", iResult);
				}
				else
				{
					std::string filename("server_files\\"); //not from class
					filename += arg;
					FILE* test = fopen(filename.c_str(), "rb");
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
						//printf("Bytes Sent on server: %ld\n", iResult);

					}
					else
					{
						//std::cout << "prepearing sender to send " << filename << std::endl;
						TcpSocket sender(new_port.c_str(), filename.c_str());
						sender.connectToServer();

						char *sendbuf = "220 OK sending file\n";
						iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ClientSocket);
							WSACleanup();
							return;
						}
						//printf("Bytes Sent on server: %ld\n", iResult);
					}
					new_port = "";
				}


			} //end of RETR



			if (command == "STOR")
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
					FILE* test = fopen(filename.c_str(), "wb");

					//std::cout << "prepearing server to recieve " << filename << std::endl;
					TcpSocket sender(new_port.c_str(), filename.c_str()); //reciever this time TODO: rename
					sender.start_recieve_data();

					char *sendbuf = "220 OK recieving file\n";
					iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(ClientSocket);
						WSACleanup();
						return;
					}
					//printf("Bytes Sent on server: %ld\n", iResult);

					new_port = "";
				}
				


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
				//printf("Bytes Sent on server: %ld\n", iResult);
			}
		}

	} while (keepAlive);

	std::cout << "End of FTP command reciever\n";
}


DWORD TcpSocket::forDataServerThread(void *param)
{
	TcpSocket *ts = (TcpSocket*)param;	
	ts->recieve_data();
	return 0;
}

void TcpSocket::recieve_data()
{
	std::cout << "Start data reciever"<<std::endl;
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
	//std::cout << "Try recieve to " << filename<< std::endl;
	FILE *output = std::fopen(filename, "wb");


	do {

		//std::cout << "fast check\n";
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received: %d\n", iResult);

			std::fwrite(recvbuf, sizeof(char), iResult, output); // to proper use of sizeof should change revbuf to dynamic
																 //std::cout << recvbuf<<"\n";
		}
		else if (iResult == 0)
			printf("\nFile recieved!\n");
		else {
			printf("Data recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			//return;
		}

	} while (iResult > 0 && keepAlive);

	fclose(output);
	std::cout << "End of reciever\n";
	stop();
	//keepAlive = false;
}




int TcpSocket::connectToServer()
{	
	hListenThread = NULL;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
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
		return 0;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		//std::cout << "connection check\n";
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
	//std::cout << "starting sender thread\n";
	hListenThread = CreateThread(NULL, 0, &forNewClientThread, this, 0, &dwListenThread);

	return 0;
}

int TcpSocket::connectToServerForCommand()
{	
	hListenThread = NULL;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 0;
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
		return 0;
	}
	
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

	
	
	hListenThread = CreateThread(NULL, 0, &forCommandClientThread, this, 0, &dwListenThread);

	while (keepAlive)
	{
	}

	return 0;
}


DWORD TcpSocket::forNewClientThread(void *param)
{
	TcpSocket *tc = (TcpSocket*)param;
	tc->sendSomeData();
	return 0;
}

DWORD TcpSocket::forCommandClientThread(void *param)
{
	TcpSocket *tc = (TcpSocket*)param;
	tc->sendSomeCommand();
	return 0;
}

void TcpSocket::sendSomeCommand()
{
	std::cout << "Sending command on port " << DEFAULT_PORT << std::endl;


	std::string command;
	std::string new_port = "";
	TcpSocket *reciever = nullptr;
	while (keepAlive)
	{

		std::getline(std::cin, command);

		char *sendbuf;
		sendbuf = new char[command.length()];
		strcpy(sendbuf, command.c_str());

		if (command == "QUIT")
		{
			keepAlive = false;			
		}

		if (command.substr(0, 4) == "RETR")
		{
			std::string tmp("client_files\\");
			if (reciever)
			{
				reciever->set_filename((tmp + command.substr(5)).c_str());
				reciever->start_recieve_data();

			}
		}

		if (command.substr(0, 4) == "STOR") // this should be before send
		{
			std::string tmp("client_files\\");

			FILE* test = fopen((tmp + command.substr(5)).c_str(), "rb");
			if (!test)
			{
				std::cout << "No such file, try again " << std::endl;
			}
			else
			{

				iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
				if (iResult == SOCKET_ERROR) {
					//printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return;
				}
				//printf("Bytes Sent on client: %ld\n", iResult);

				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0) {
					//printf("Bytes received on client: %d\n", iResult);
					//std::string tmp(recvbuf);
					//std::cout << recvbuf << tmp<<"test\ntest"<<std::endl;
					std::cout.write(&recvbuf[0], iResult) << "\n";
				}

				if (recvbuf[0] == '2')
				{

					if (reciever)
					{
						reciever->set_filename((tmp + command.substr(5)).c_str());
						reciever->connectToServer(); //this is a sender this time TODO: rename reciever to localSocket					
					}

				}
				else
				{
					if (reciever)
					{

						reciever->stop();
					}

				}
				reciever = nullptr;
			}
			continue;
		}


		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			//printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return;
		}
		//printf("Bytes Sent on client: %ld\n", iResult);

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//printf("Bytes received on client: %d\n", iResult);				
			std::cout.write(&recvbuf[0], iResult) << "\n";
		}


		if (command.substr(0, 4) == "RETR")
		{
			/*std::string tmp("client_files\\");
			if (reciever)
			{
				reciever->set_filename((tmp + command.substr(5)).c_str());
				reciever->start_recieve_data();
				
			}
			//std::cout << "OK" << std::endl;

			
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return;
			}
			//printf("Bytes Sent on client: %ld\n", iResult);

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				//printf("Bytes received on client: %d\n", iResult);
				std::cout << recvbuf << std::endl;
			}
			*/

			if (recvbuf[0] != '2')
			{
				
				if (reciever)
				{			

					reciever->stop();
					reciever = nullptr;
				}
				
			}
			continue;

		}

	
		if (command == "PASV")
		{
			new_port = "";
			/*iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				//printf("Bytes received on client: %d\n", iResult);
				std::cout << recvbuf << std::endl;
			}*/
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

				reciever = new TcpSocket(new_port.c_str());
			}
		}
		

		/*if (command == "LIST")
		{
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				//printf("Bytes received on client: %d\n", iResult);				
				std::cout.write(&recvbuf[0], iResult) << "\n";
			}
		}*/
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

	stop();

}

void TcpSocket::sendSomeData()
{
	std::cout << "Start sender";
	//std::cout << "Sending data\nOn port " << DEFAULT_PORT << std::endl << "from " << filename << std::endl;
		
	FILE *input = std::fopen(filename, "rb");
		

	if (ferror(input))
		std::cout << "File cant be opened!\n"; // сообщить об этом
	else
	{
		while (!feof(input))
		{
			//std::fgets(sendbuf, DEFAULT_BUFLEN, input);

			//char buffer[DEFAULT_BUFLEN];
			//std::cout << "chech2" << std::endl;
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

	stop();
}

void TcpSocket::stop()
{
	
	closesocket(ClientSocket);
	closesocket(ConnectSocket);
	WSACleanup();
	std::cout << "Socket stopped"<<std::endl;
	keepAlive = false;
}