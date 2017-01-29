// VSFTP.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
//#include "TcpServer.h"
//#include "TcpClient.h"
#include "TcpSocket.h"
int main()
{
	/*TcpServer server("21");
	TcpClient client("21");
	//server.start();
	server.start_control();
	client.connectToServerForCommand();*/

	TcpSocket server("21");
	TcpSocket client("21");

	server.start_recieve_control();
	//Sleep(500);
	client.connectToServerForCommand();

	
	system("pause");
	std::cout << "End of main";
	//getchar();	
    return 0;
}

