#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 8800
#define CACHE_WID 12
#define CACHE_LENGTH DEFAULT_BUFLEN
#define DEFAULT_PORT "27015"
//端口号和空间大小的宏定义

#define CACHE_WID 100
#define LOCK 0
#define UNLOCK 1
#define NOT_EMPTY 0
#define EMPTY 1
#define NOT_FULL 0
#define FULL 1
#define SUCCESS 1
#define FAIL 0
#define DONE 0
#define NOT_DONE 1
//多线程控制量的宏定义


class Cache
{
  public:
    FILE* fp;                  //文件指针
    int header;                //前指针标识
    int end;                   //尾指针标识
    char cacheBuf[CACHE_WID][CACHE_LENGTH]; //缓冲区数组
    int available;             //缓冲区读写锁
    int empty;                 //缓冲区是否为空的标识
    int full;                  //缓冲区是否满的标识
    int round;                 //header和end相对位置的标识
    int lastTimeSize;          //最后一个文件块的大小
    unsigned long transTime;   //文件整块传输的次数
    int pipeLineCond;

    void zeroSpace()
    //给数组清零
    {
        for (int i = 0; i < CACHE_WID; i++)
            memset(cacheBuf[i], '/0', CACHE_LENGTH);
    }

    int judgeEmpty()
    //判断缓冲区是否为空
    //空则返回EMPTY 否则返回NOT_EMPTY
    {
        if(round == 0 && header == end) 
        {
            empty = EMPTY;
            return EMPTY;
        }
        else 
        {
            empty = NOT_EMPTY;
            return NOT_EMPTY;
        }
    }

    int judgeFull()
    //判断缓冲区是否为满
    //满则返回FULL 否则返回NOT_FULL
    {
       if((round == 1 && header == end)
        || (header == CACHE_WID  - 1 && end == 0)) 
       {
           full = FULL;
           return FULL;
       }
       else 
       {
           full = NOT_FULL;
           return NOT_FULL;
       }
    }


};



DWORD WINAPI readCache(LPVOID para);



int __cdecl main(void) 
{

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\a.pdf";
    FILE* fp = fopen(path.c_str(), "wb");
    unsigned long transCount = 0;
    int lastTimeSize = 0;
    unsigned long curRecvCount = 0;
    //文件传输相关参数

    Cache cache;
    cache.fp = fp;
    cache.header = cache.end = cache.round = 0;
    cache.empty = EMPTY;
    cache.full = NOT_FULL;
    cache.available = UNLOCK;
    cache.zeroSpace();
    cache.pipeLineCond = NOT_DONE;
    //类对象参数初始化
    //---------------------------------------------------------

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    //对Socket版本进行限制
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    //先清零
    hints.ai_family = AF_INET;//限定为ipv4
    hints.ai_socktype = SOCK_STREAM;//限定为stream模式传送
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;//记录进入连接的ip地址

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    //把服务器端口、地址和配置参数写入result hints相当于一个中介
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //用ListenSocket来接收初始配置socket的各种信息
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket

    //关于bind()
    //一个socket的名字有地址族 主机地址 端口号组成
    //socket函数指定一个地址族 bind函数指定地址和端口号
    //因此bind（）只能用于还没有连接的socket 只能在connect()或listen()之前调用
    //一个socket只能调用一次bind
    //调用了一次bind之后socket就不能再更改了
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    //通常监听部分会写成循环 直到有连接接入为止
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    //当有连接发生时 调用accept 但是是单线程的
    //一说当连接发生accept被调用后 服务器程序会开一个新线程来处理次连接
    //服务器程序接着继续顺序执行或者继续监听其他连接
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    char countStr[30] = {0};


    //---------------------------------------------------------------------------------------------
    //-------------------------接收核心部分---------------------------------------------------------
    //---------------------------------------------------------------------------------------------

    iResult = recv(ClientSocket, countStr, 30, 0);
    if (iResult < 0)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        Sleep(5000);
        WSACleanup();
        return 1;
    }
    transCount = cache.transTime = (unsigned long)atoi(countStr);
    //接收发送次数并初始化信息
    //----------------------------------------------------------------
    memset(countStr, '\0', 30);
    iResult = recv(ClientSocket, countStr, 30, 0);
    lastTimeSize = cache.lastTimeSize = (int)atoi(countStr);
    //接收最后一个块的尺寸
    //----------------------------------------------------------------

    HANDLE pipeline = CreateThread(NULL, 0, readCache,
                                   (void *)&cache, 0, NULL);
    CloseHandle(pipeline);
    cout << "文件传输次数 " << transCount << endl;
    cout << endl << "LTS: " << cache.lastTimeSize << endl;
    Sleep(10000);
    //线程启动 

    while(curRecvCount < transCount)
    {
        while(cache.judgeFull() == NOT_FULL && curRecvCount < transCount)
        //只要给空位就疯狂的输出
        {
            // cout << curRecvCount << " " << cache.header << " " << cache.end << " " << cache.round;
            iResult = recv(ClientSocket, cache.cacheBuf[cache.header]
                            , CACHE_LENGTH, 0);
            curRecvCount++;
            if(cache.header == CACHE_WID - 1)
            {
                cache.round = 1;
                cache.header = 0;
            }
            else cache.header ++;
            //常规接收
            //改计数量
            //改读写标记位
            //--------------------------------------------------------

            if(iResult < 0)
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                Sleep(5000);
                WSACleanup();
                return 1;
            }
        }
    }
    //常规接收完毕
    //----------------------------------------------------------------

    memset(cache.cacheBuf[cache.header], '\0', CACHE_LENGTH);
    while(cache.judgeFull() == FULL);
    //如果不空就一直等着呗
    iResult = recv(ClientSocket, cache.cacheBuf[cache.header], 
                    lastTimeSize, 0);
    if (cache.header == CACHE_WID - 1)
    {
        cache.round = 1;
        cache.header = 0;
    }
    else
        cache.header++;
    //最后一次接收
    //改计数量
    //改读写标记位
    //socket此时是生产者 不改读写标记位是不可能的
    //--------------------------------------------------------

    if (iResult < 0)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        Sleep(5000);
        WSACleanup();
        return 1;
    }

    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    while(cache.pipeLineCond == NOT_DONE) ;
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        Sleep(5000);
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    Sleep(5000);
    closesocket(ClientSocket);
    WSACleanup();


    return 0;
}

DWORD WINAPI readCache(LPVOID para)
{
    Cache* cachePtr = (Cache*) para;
    
    unsigned long count = 0;

    while(count < cachePtr -> transTime )
    {
        if (cachePtr -> judgeEmpty() == NOT_EMPTY && count < cachePtr -> transTime)
        {
            fwrite(cachePtr->cacheBuf[cachePtr->end], CACHE_LENGTH, 1, cachePtr->fp);
            if(cachePtr -> end == CACHE_WID - 1) cachePtr -> round = 0;
            cachePtr -> end = (cachePtr -> end + 1) % CACHE_WID;
            cout << "write: " << count  << " end " << cachePtr->end << endl;
            count ++;
        }
        //-----------------------------------------------------
        //适当控制写入文件的速度 为接收端提供更多时间 每次读完都要修改end
        //循环结束调用两个函数 修改空满标志量
        //----------------------------------------------------
    }
    //-------------------整块传送完了------------------------------
    
    while(cachePtr -> judgeEmpty() == EMPTY) ;//等待最后一片写进来
    fwrite(cachePtr->cacheBuf[cachePtr->end], cachePtr->lastTimeSize,
           1, cachePtr->fp);
    fclose(cachePtr->fp);
    cachePtr->pipeLineCond = DONE;
    cout << "done save" << endl;
    return 0;
}