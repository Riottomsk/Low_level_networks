// VSTcp.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include "TcpServer.h"
#pragma comment (lib, "Ws2_32.lib")



int main()
{
	TcpServer server;
	server.start();
	std::cout << "End of main";
	getchar();
    return 0;
}

