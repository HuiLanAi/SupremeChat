#pragma once
#define WIN32_LEAN_AND_MEAN

#include "winsock2.h"
#include "ws2tcpip.h"
#include "string"
#include "iostream"
#include "cstring"
#include "windows.h"
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define LOGIN_FLAG '1'
#define FRIMES_FLAG '2'
#define INQU_FLAG '3'

#define DEFAULT_PORT "27015"
//老版本中用于收发一体的端口
//在v3版本中只用于服务器的收和客户端的发
#define SUB_PORT "27016"
//v3多线程版本中的子端口
//用于服务器的发和客户端的收



void getLoginInfo(string* sendMes)
/* 函数功能：用户数据输入，并按照报文格式写入输入的字符串指针
   函数输入：用来存放输入的指针 */
{
    (*sendMes).clear();
    (*sendMes) += "1";
    string temp, temp1;
    cout << "请输入用户名：" << endl;
    getline(cin, temp);
    temp.append("|");
    cout << "请输入密码： " << endl;
    getline(cin, temp1);
    temp.append(temp1);
    temp.append("|");
    temp1.clear();
    cout << "请输入IP地址： " << endl;
    getline(cin, temp1);
    temp.append(temp1);
    *sendMes = LOGIN_FLAG;
    (*sendMes).append(temp);
    system("pause");
    // system("cls");
}

int firstTimeLogin(string* ipAddrServer, string loginInfo, addrinfo* hint, addrinfo **result)
/* 发送初始登录信息函数 报文格式为1用户名|密码|IP地址
   返回值-1为地址错误
   返回值-2为SOCKET创建错误 
   返回值-3为连接服务器失败
   返回值-4为发送登录信息失败
   返回值0 登录成功
   返回值1 用户名或密码错误
   返回值2 不存在该用户*/
{
    int iRes = getaddrinfo((*ipAddrServer).c_str(), "27015", hint, result);
    if(iRes != 0){
        cout << "地址错误！ " << iRes << endl;
        // WSACleanup();
        return -1;
    }
    
    struct addrinfo* ptr = *result;
    SOCKET ConnectSocket;

    for (; ptr != NULL; ptr = ptr->ai_next)
    {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                                      ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("SOCKET创建错误: %ld\n", WSAGetLastError());
            // WSACleanup();
            return -2;
        }

        iRes = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iRes == SOCKET_ERROR) {
            //一旦失败 此处最好再尝试连接一次
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("连接失败!\n");
        // WSACleanup();
        return -3;
    }

    iRes = send(ConnectSocket, loginInfo.c_str(), loginInfo.length(), 0);
    //发送初始登录信息

    if (iRes == SOCKET_ERROR)
    {
        printf("发送失败: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        // WSACleanup();
        return -4;
    }

    char recvCache[DEFAULT_BUFLEN] = {0};
    iRes = recv(ConnectSocket, recvCache, DEFAULT_BUFLEN, 0);
    string mesFromCache = recvCache;
    mesFromCache[0] = ' ';
    if(iRes > 0)
    {
        if(recvCache[0] == '0') {cout << "登录成功" << endl; cout << mesFromCache << endl;return 0;}
        if(recvCache[0] == '1') {cout << "用户名或密码错误" << endl; return 1;}
        if(recvCache[0] == '2') {cout << "不存在该用户" << endl; return 2;}
    }
    else if (iRes == 0) printf("连接已关闭\n");
    else printf("连接失败: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    system("pause");
    // system("cls");
}

int sendMessageToServer(string* ipAddrServer, string message, 
                addrinfo* hint, addrinfo **result)
/* 函数功能：发送普通对话消息 
   报文格式为 flag发送者ID|接受者ID|发送时间|信息*/
{
    int iRes = getaddrinfo((*ipAddrServer).c_str(), "27015", hint, result);
    if(iRes != 0){
        cout << "地址错误！ " << iRes << endl;
        // WSACleanup();
        return -1;
    }
    
    struct addrinfo* ptr = *result;
    SOCKET ConnectSocket;

    for (; ptr != NULL; ptr = ptr->ai_next)
    {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                                      ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("SOCKET创建错误: %ld\n", WSAGetLastError());
            // WSACleanup();
            return -2;
        }

        iRes = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iRes == SOCKET_ERROR) {
            //一旦失败 此处最好再尝试连接一次
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("连接失败!\n");
        // WSACleanup();
        return -3;
    }

    iRes = send(ConnectSocket, message.c_str(), message.length(), 0);
    //发送初始登录信息

    if (iRes == SOCKET_ERROR)
    {
        printf("发送失败: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        // WSACleanup();
        return -4;
    }

    char recvCache[DEFAULT_BUFLEN] = {0};
    iRes = recv(ConnectSocket, recvCache, DEFAULT_BUFLEN, 0);
    if(iRes > 0)
    {
        cout << "发送成功" << endl;
        cout << recvCache << endl;
    }
    else if (iRes == 0) printf("连接已关闭\n");
    else printf("连接失败: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
}

int fakeListen(string* ipAddrServer, string userid, addrinfo* hint, addrinfo **result)
/* 探查函数
   每次循环都要想服务器发送询问报文
   服务器接收到flag位询问报文的消息时
   检查消息缓存
   报文格式为 3.userid*/
{
    int iRes = getaddrinfo((*ipAddrServer).c_str(), "27015", hint, result);
    if(iRes != 0){
        cout << "地址错误！ " << iRes << endl;
        // WSACleanup();
        return -1;
    }
    
    struct addrinfo* ptr = *result;
    SOCKET ConnectSocket;

    for (; ptr != NULL; ptr = ptr->ai_next)
    {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                                      ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("SOCKET创建错误: %ld\n", WSAGetLastError());
            // WSACleanup();
            return -2;
        }

        iRes = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iRes == SOCKET_ERROR) {
            //一旦失败 此处最好再尝试连接一次
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("连接失败!\n");
        // WSACleanup();
        return -3;
    }

    userid = "3" + userid;

    iRes = send(ConnectSocket, userid.c_str(), userid.length(), 0);
    //发送初始登录信息

    if (iRes == SOCKET_ERROR)
    {
        printf("发送失败: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        // WSACleanup();
        return -4;
    }

    char recvCache[DEFAULT_BUFLEN] = {0};
    iRes = recv(ConnectSocket, recvCache, DEFAULT_BUFLEN, 0);
    if(iRes > 0)
    {
        cout << recvCache << endl;
        return 41;
    }
    else if (iRes == 0) printf("连接已关闭\n");
    else printf("连接失败: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    system("pause");
    // system("cls");
}


class InqClitInfo
//InquiryClientInfo
//专门为inqury子线程设计的类
{
    public:
     InqClitInfo(int inputName, string inputAddr)
     {
         userName = inputName;
         addr = inputAddr;
     }

     int userName;//用户名
     string addr;//服务器IP地址
};


#define INQUIRY_PORT "27016"
//询问报文端口


int sendInquiryMessage(string* ipAddrServer, string message, 
                addrinfo* hint, addrinfo **result)
/* 函数功能：发送普通对话消息 
   报文格式为 flag发送者ID
   flag标志位为3*/
{
    int iRes = getaddrinfo((*ipAddrServer).c_str(), INQUIRY_PORT, hint, result);
    if(iRes != 0){
        cout << "地址错误！ " << iRes << endl;
        // WSACleanup();
        return -1;
    }
    
    struct addrinfo* ptr = *result;
    SOCKET ConnectSocket;

    for (; ptr != NULL; ptr = ptr->ai_next)
    {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
                                      ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("SOCKET创建错误: %ld\n", WSAGetLastError());
            // WSACleanup();
            return -2;
        }

        iRes = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iRes == SOCKET_ERROR) {
            //一旦失败 此处最好再尝试连接一次
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("连接失败!\n");
        // WSACleanup();
        return -3;
    }

    iRes = send(ConnectSocket, message.c_str(), message.length(), 0);
    //发送初始登录信息

    if (iRes == SOCKET_ERROR)
    {
        printf("发送失败: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        // WSACleanup();
        return -4;
    }

    char recvCache[DEFAULT_BUFLEN] = {0};

    iRes = recv(ConnectSocket, recvCache, DEFAULT_BUFLEN, 0);
    cout << "client_user 350 iRes " << iRes << endl;
    

    if(iRes > 0)
    {
        cout << "嗅探成功" << endl;
        string recvStr(recvCache);
        if(recvStr != " ")
            cout << recvCache << endl;
    }
    else if (iRes == 0) printf("连接已关闭\n");
    else 
    {
        printf("连接失败: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
    }
   
}




DWORD WINAPI inquiry(LPVOID clientInfo)
//inquiry线程
//报文格式为"3用户名"
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    char sendbuf[DEFAULT_BUFLEN] = {0};
    char recvbuf[DEFAULT_BUFLEN] = {0};
    int funcRes;
    int recvbufLen = DEFAULT_BUFLEN;

    InqClitInfo* info = (InqClitInfo*) clientInfo;

    string ipAddrServer = info -> addr;
    //用线程类参数得到服务器的ip地址

    funcRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (funcRes != 0) {
        printf(" inquiry WSAStartup failed with error: %d\n", funcRes);
    }
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    //从服务器端返回的地址协议是非指定的
    //AF_INET指定ipv4
    //AF_INET6指定ipv6
    hints.ai_socktype = SOCK_STREAM;//STREAM模式
    hints.ai_protocol = IPPROTO_TCP;//TCP协议
    //配置SOCKET基本参数


    while (true)
    //发送部分
    {
        string userNameStr = to_string(info -> userName);

		cout << "client_user 407 " << endl;

        // fakeListen(&ipAddrServer, userNameStr, &hints, &result);
        string input = "";
        //用于发送的字符串
        input += "3";
        input =input + userNameStr;
        //生成报文

        //这个地方要重新设置一下端口号
        funcRes = sendInquiryMessage(&ipAddrServer, input, &hints, &result);

        Sleep(80);
        //每100毫秒执行一次到服务器的问询
    }

}