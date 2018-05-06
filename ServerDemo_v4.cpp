/* 服务器端版本4
    文件传输demo
    可能要新建一种报文类型
    暂时还使用27015端口来传文件 */

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>


#include <stdio.h>
#include "iostream"
#include "fstream"
#include "cstring"
#include "string"
using namespace std;

#include "OutOfNetWork.h"
#include "User.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 8188
//不知道改这个数组的大小有没有用

#define DEFAULT_PORT "27015"
//老版本中用于收发一体的端口
//在v3版本中只用于服务器的收和客户端的发
#define INQUIRY_PORT "27016"
//v3多线程版本中的子端口
//用于服务器的发和客户端的收


int __cdecl main(void)
{
    
    vector<User> userData;
    vector<Message> mesCache;
    mesCache.clear();
    MesCacheInfo mesCacheInfo((int)(mesCache.size()), &mesCache);
    //把mesCache和其大小一起封装    

    //清空数据库

    // 可能要开一个多线程绑定一个新的监听端口
    //HANDLE inquiryThread = CreateThread(NULL, 0, handleInquiry, (void*) &mesCacheInfo, 0, NULL);
    //传入一个封装信息类

    initialize(&userData);

    WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	char recvBuf[DEFAULT_BUFLEN] = {0};

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

    char fileFlag = 0;
    //文件传输标识符 不得不写在大循环外面
    char tempFile[DEFAULT_BUFLEN];
    memset(tempFile, '\0', DEFAULT_BUFLEN);
    //当前回合接收了文件 把recvBuf的值转移进来
    //下回合recvBuf接受了文件大小
    //再把tempFile写入
    

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


        // for (int i = 0; i < DEFAULT_BUFLEN; i++)
        //     recvBuf[i] = 0;
        //清空接收池

        memset(recvBuf, 0, DEFAULT_BUFLEN);//recvBuf清零

        char fileSizeStr [50] = {0};
        //用于临时保存文件大小
        unsigned int fileSize = 0;

        iResult = recv(ClientSocket, recvBuf, DEFAULT_BUFLEN, 0);
        cout << recvBuf << endl;

        if(recvBuf[2] != '|' && fileFlag == 0) fileFlag = 1;
        if(recvBuf[2] == '|') fileFlag = 0;

        if (iResult > 0)
        //这里为了测试文件传输 注释掉了很多无关动作 不然文件传输没法运行
        {
            // printf("%s\n", recvBuf);
            string recvBufToStr = recvBuf;
            string retVal = "发送成功";

            //转移recvBuf

            if(!fileFlag)
            retVal = OutOfNetwork(recvBufToStr, userData, &mesCacheInfo);
            //为了测试文件传输 不得不把OutOfNetwork()里的正常报文情况注释掉了

            // if(fileFlag == 1)
            // {
            //     cout << (int) recvBuf[i] << endl;
            // }


            cout << "反馈消息： " << retVal << endl;

            int res = send(ClientSocket, retVal.c_str(), retVal.length(), 0);
            // 马上要修改 要适应多线程的修改 此处只返回发送成功的信号就行了 要改动OutOfNetwork函数

            if (fileFlag == 1)
            {
                for(int i = 0; i < DEFAULT_BUFLEN; i ++)
                {
                    tempFile[i] = recvBuf[i];
                }
                cout << "tempFile: \n" << tempFile << endl;
            }

            //fileFlag为1 当前接收的是文件
            //fileFlag为2 当前要接收文件大小
            
            if (res == SOCKET_ERROR)
            {
                printf("发送失败: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

            // if (fileFlag)
            // //区分文件报文还是消息/登录报文
            // //下下之策 为了赶时间做demo
            // {
            //     string retVal = "接收到文件大小";
            //     int fres = recv(ClientSocket, fileSizeStr, 50, 0);
            //     send(ClientSocket, retVal.c_str(), retVal.length(), 0);
            //     fileSize = atoi(fileSizeStr);
            //     cout << "文件大小报文 " << fileSizeStr << endl;
            //     memset(fileSizeStr, '\0', 50);
            // }
        }
        if (iResult == 0)
        //这里原来是else if (iResult == 0)
        {
            printf("连接即将关闭");
            //return 1;
        }

 
        // else
        // {
        //     printf("连接发生错误", WSAGetLastError());
        //     closesocket(ClientSocket);
        //     WSACleanup();
        //     return 1;
        // }

        // if (iResult == SOCKET_ERROR)
        // {
        //     printf("shutdown failed with error: %d\n", WSAGetLastError());
        //     closesocket(ClientSocket);
        //     WSACleanup();
        //     return 1;
        // }
        // cout << recvBuf << endl;

        if (fileFlag == 2)
        {
            fileSize = atoi(recvBuf);
            //cout << "文件大小报文 " << tempFile << endl;
            FILE* fout = fopen("C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\ConsoleApplication1\\Debug\\10.png", "wb");
            fwrite(tempFile, fileSize, 1, fout);
            fclose(fout);
            memset(tempFile, '\0', DEFAULT_BUFLEN);
            memset(recvBuf, '\0', DEFAULT_BUFLEN);
        }

        if(fileFlag == 1) fileFlag ++;
        // cleanup
        // closesocket(ClientSocket);
    }
}