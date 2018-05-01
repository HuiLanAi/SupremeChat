#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>


#include <stdio.h>
#include "iostream"
#include "string"
using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//对Socket版本进行限制
	if (iResult != 0) {
		printf("Socket版本故障: %d\n", iResult);
		return 1;
	}

	cout << "Socket版本初始化完毕" << endl;

	ZeroMemory(&hints, sizeof(hints));
	//先清零
	hints.ai_family = AF_INET;//限定为ipv4
	hints.ai_socktype = SOCK_STREAM;//限定为stream模式传送
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;//记录进入连接的ip地址

	// sockaddr_in temp;
	// temp.sin_family = AF_INET;
	// temp.sin_port = htons(27015);
	// temp.sin_addr.S_un.S_addr = INADDR_ANY;S


								// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	//把服务器端口、地址和配置参数写入result hints相当于一个中介
	if (iResult != 0) {
		printf("参数初始化故障: %d\n", iResult);
		WSACleanup();
		return 1;
	}


	cout << "Socket参数初始化完毕" << endl;


	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//用ListenSocket来接收初始配置socket的各种信息
	if (ListenSocket == INVALID_SOCKET) {
		printf("监听初始化故障: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	cout << "Socket监听初始化完毕" << endl;
	
	// Setup the TCP listening socket

	//关于bind()
	//一个socket的名字有地址族 主机地址 端口号组成
	//socket函数指定一个地址族 bind函数指定地址和端口号
	//因此bind（）只能用于还没有连接的socket 只能在connect()或listen()之前调用
	//一个socket只能调用一次bind
	//调用了一次bind之后socket就不能再更改了
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);


	if (iResult == SOCKET_ERROR) {
		printf("绑定故障: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Socket绑定初始化完毕" << endl;

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);

	//通常监听部分会写成循环 直到有连接接入为止
	if (iResult == SOCKET_ERROR) {
		printf("监听失败: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	while(1)
	{
		cout << "waiting..." << endl;
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			cout << "接收失败" << endl;
			continue;
		}
		
		cout << "收到一个链接: ";
		break;
	}


	// Accept a client socket
	// ClientSocket = accept(ListenSocket, NULL, NULL);

	// cout << "oop  " << iResult << endl;
	
	//当有连接发生时 调用accept 但是是单线程的
	//一说当连接发生accept被调用后 服务器程序会开一个新线程来处理次连接
	//服务器程序接着继续顺序执行或者继续监听其他连接
	if (ClientSocket == INVALID_SOCKET) {
		printf("接收失败: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//这个地方可能要去掉
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do {
		for (int i = 0; i < DEFAULT_BUFLEN; i++)
			recvbuf[i] = 0;
		//清空接收池
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		string check_string = recvbuf;
		if(check_string == "  ") break;
		if (iResult > 0) {
			printf("%s\n", recvbuf);
			iSendResult = send(ClientSocket, "0", iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("发送失败: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (iResult == 0)
			printf("连接即将关闭n");
		else {
			printf("连接发生错误", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		cout << "正在等待接收" << endl;

	} while (iResult > 0);

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