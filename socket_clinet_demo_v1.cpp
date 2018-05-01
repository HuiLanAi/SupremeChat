/* 客户端版本1
   在确定服务器无法同时连接两个监听的情况下
   通过初始登录验证 以及每发送一条报文就断开连接的方式解决问题
   关键在于服务器端登录状态的更新 和初始socket参数不变 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "string"
#include "iostream"
#include "client_user.h"
using namespace std;


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main()
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    char sendbuf[DEFAULT_BUFLEN] = {0};
    char recvbuf[DEFAULT_BUFLEN] = {0};
    int funcRes;
    int recvbufLen = DEFAULT_BUFLEN;

    cout << "请输入服务器当前IP地址" << endl;
    string ipAddrServer;
    getline(cin, ipAddrServer);
    system("pause");
    system("cls");

    funcRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (funcRes != 0) {
        printf("WSAStartup failed with error: %d\n", funcRes);
        return 1;
    }
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    //从服务器端返回的地址协议是非指定的
    //AF_INET指定ipv4
    //AF_INET6指定ipv6
    hints.ai_socktype = SOCK_STREAM;//STREAM模式
    hints.ai_protocol = IPPROTO_TCP;//TCP协议
    //配置SOCKET基本参数

    string input;
    input.clear();
    //用户输入字符串
    
    do
    {
        getLoginInfo(&input);
        funcRes = firstTimeLogin(&ipAddrServer, input, &hints, &result);
    } while(funcRes != 0);


    while (input != "2  ")
    {
        input.clear();
        string temp;
        cout << "请键入新消息" << endl;
        getline(cin, temp);
        input += "2";
        input += temp;
        funcRes = sendMessageToServer(&ipAddrServer, input, &hints, &result);
        system("pause");
    }
}
