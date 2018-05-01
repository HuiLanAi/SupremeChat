/* 客户端版本1
   在确定服务器无法同时连接两个监听的情况下
   通过初始登录验证 以及每发送一条报文就断开连接的方式解决问题
   关键在于服务器端登录状态的更新 和初始socket参数不变 */

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

	char recvBuf[DEFAULT_BUFLEN];

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

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	//把服务器端口、地址和配置参数写入result hints相当于一个中介
	if (iResult != 0) {
		printf("参数初始化故障: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	cout << "Socket参数初始化完毕" << endl;


    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//用ListenSocket来接收初始配置socket的各种信息
	if (ListenSocket == INVALID_SOCKET) {
		printf("监听初始化故障: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	cout << "Socket监听初始化完毕" << endl;

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);


	if (iResult == SOCKET_ERROR) {
		printf("绑定故障: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Socket绑定初始化完毕" << endl;

	// freeaddrinfo(result);
    //这行怎么改我也不确定啊 2018.3.17 13:16

    while (1)
    {
        iResult = listen(ListenSocket, SOMAXCONN);

        //通常监听部分会写成循环 直到有连接接入为止
        if (iResult == SOCKET_ERROR)
        {
            printf("监听失败: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        //循环监听

        cout << "正在等待接入..." << endl;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
		{
			cout << "接收失败" << endl;
			continue;
		}
		cout << "收到一个消息: " << endl;

        if (ClientSocket == INVALID_SOCKET)
        {
            printf("接收失败: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            // WSACleanup();
            return 1;
        }

        //这个地方可能要去掉
	    //closesocket(ListenSocket);

        for (int i = 0; i < DEFAULT_BUFLEN; i++)
			recvBuf[i] = 0;
        //清空接收池

		iResult = recv(ClientSocket, recvBuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("%s\n", recvBuf);
            int res = send(ClientSocket, "0", iResult, 0);
            if (res == SOCKET_ERROR)
            {
                printf("发送失败: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)
            printf("连接即将关闭n");
        else
        {
            printf("连接发生错误", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        closesocket(ClientSocket);
    }
}