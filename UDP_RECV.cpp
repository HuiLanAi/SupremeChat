#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include "string"
#include "cstring"
#include "iostream"
#include "fstream"
#include "windows.h"
using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 400
#define CACHE_LENGTH 400
#define CACHE_WID 42
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

/* 
// 先初始化一个类对象
// 在接收到第一个报文后设置transCount
// 在接收到倒数第二个报文后设置lastTimeSize
 
//  每次接收直接复制到cache的二维数组里
//  不用设置读写锁 只要不超过header就可以*/

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
       if(round == 1 && header == end) 
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

DWORD WINAPI writeFile(LPVOID para);


int main()
{

    int iResult = 0;
    WSADATA wsaData;
    SOCKET RecvSocket;
    sockaddr_in RecvAddr;
    unsigned short Port = 27015;
    char RecvBuf[BUFLEN] = {0};
    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof (SenderAddr);
    //网络初始化相关参数

    unsigned long transCount = 0;
    int lastTimeSize = 0;
    unsigned long curRecvCount = 0;
    //文件传输相关参数

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\a.pdf";
    FILE* fp = fopen(path.c_str(), "wb");
    //文件写相关参数

    //-----------------------------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error %d\n", iResult);
        return 1;
    }
    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error %d\n", WSAGetLastError());
        return 1;
    }
    //-----------------------------------------------
    // Bind the socket to any address and the specified port.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(RecvSocket, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
    if (iResult != 0) {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return 1;
    }
    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.

    //----------------------------------------------------------------
    //----------------接收的核心模块-------------------------------------
    //-----------------------------------------------------------------
    wprintf(L"Receiving datagrams...\n");

    DWORD startTime = 0;
    DWORD endTime = 0;
    //计时器变量
    //-------------------------------------------------------

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


    while(1)
    {
        if(curRecvCount == 0)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0, 
                                (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            startTime = GetTickCount();
            transCount = (unsigned int ) atoi(RecvBuf);
            cout << "传输次数为: " << transCount << endl;
            cache.transTime = transCount;
            memset(RecvBuf, '\0', 100);
            //初始化类里的值
            //接收传输次数
            //注意此处用RECVBUF来接收
            //--------------------------------------------

            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"first recvfrom failed with error %d\n", WSAGetLastError());
                closesocket(RecvSocket);
            }

            HANDLE pipeLine = CreateThread(NULL, 0, writeFile, 
                                (void*)&cache, 0, NULL);


            iResult = recvfrom(RecvSocket, cache.cacheBuf[cache.header], CACHE_LENGTH, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            if(cache.header == CACHE_WID - 1) cache.round = 1;
            cache.header = (cache.header + 1) % CACHE_WID;
            //直接赋值到缓冲区
            //----------------------------------------------------------------
            
            cache.judgeEmpty();
            cache.judgeFull();
            //修改缓冲区读写参数量
            
            curRecvCount ++;
    
        }
        //-------------------------------------------------------
        //curRecvCount为0时 先接收传送次数 再接收第一个块
        //---------------------------------------------------------

        else if(curRecvCount < transCount)
        {
            iResult = recvfrom(RecvSocket, cache.cacheBuf[cache.header], CACHE_LENGTH, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            if(cache.header == CACHE_WID - 1)
            {
                cache.round = 1;
                cache.header = 0;
            }
            else cache.header++;
            //直接赋值到缓冲区
            //----------------------------------------------------------------

            cache.judgeEmpty();
            cache.judgeFull();
            //修改缓冲区读写参数量

            // cout << curRecvCount << endl;            
            
            curRecvCount++;
        }
        //---------------------------------------------------------
        //连续接收整块
        //---------------------------------------------------------

        else if(curRecvCount == transCount)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            lastTimeSize = atoi(RecvBuf);
            cache.lastTimeSize = lastTimeSize;
            cache.available = LOCK;//表示lastTimeSize已经接受到并被赋值
            //接受最后一块的大小并修改缓冲区对应的值
            //注意此处用RECVBUF来接收
            //------------------------------------------------------------------

            memset(cache.cacheBuf[cache.header], '\0', CACHE_LENGTH);
            iResult = recvfrom(RecvSocket, cache.cacheBuf[cache.header], lastTimeSize, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            if(cache.header == CACHE_WID - 1) cache.round = 1;
            cache.header = (cache.header + 1) % CACHE_WID;
            cache.judgeEmpty();
            cache.judgeFull();
            //清空接收区 修改header 修改缓冲区读写参数量
            //-----------------------------------------------------------------

            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
                closesocket(RecvSocket);
            }

            endTime = GetTickCount();
            break;
        }
        //---------------------------------------------------------
        //接收和写入最后一块
        //---------------------------------------------------------
        else;

        if (iResult == SOCKET_ERROR)
        {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
            closesocket(RecvSocket);
        }
    }

    
    //---------------------------------------------------------------------
    //------------核心传输部分-------------------------------------------
    //-----------------------------------------------------------------
    

    //-----------------------------------------------
    // Close the socket when finished receiving datagrams
    wprintf(L"Finished receiving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    //-----------------------------------------------
    // Clean up and exit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    
    while(cache.pipeLineCond == NOT_DONE) ;

    cout << "耗时 " << (float)(endTime - startTime) / 1000 << endl;
    cin >> path;

    

    return 0;
}



DWORD WINAPI writeFile(LPVOID para)
/* 
初始化各种参数
判断是不是为空
不为空则直接索引数组指针写入
修改满空量 */
{
    Cache* cachePtr = (Cache*) para;
    
    unsigned long count = 0;
    
    while(count < cachePtr -> transTime )
    {
        while (cachePtr -> judgeEmpty() == NOT_EMPTY)
        {
            fwrite(cachePtr->cacheBuf[cachePtr->end], CACHE_LENGTH, 1, cachePtr->fp);
            count ++;
            if(cachePtr -> end == CACHE_WID - 1) cachePtr -> round = 0;
            cachePtr -> end = (cachePtr -> end + 1) % CACHE_WID;
            cachePtr -> judgeFull();
        }
        //-----------------------------------------------------
        //只要不为空就使劲的写啊写 每次读完都要修改end
        //循环结束调用两个函数 修改空满标志量
        //----------------------------------------------------
    }
    //-------------------整块传送完了------------------------------
    
    while(cachePtr -> judgeEmpty() == EMPTY || cachePtr -> available == UNLOCK);//等待最后一片写进来

    fwrite(cachePtr -> cacheBuf[cachePtr -> end], cachePtr -> lastTimeSize, 
            1, cachePtr -> fp);
    fclose(cachePtr -> fp);
    //---------------------------------------------------------------
    //最后一片的写入
    //--------------------------------------------------------------

    cachePtr -> pipeLineCond = DONE;
    //通知main函数可以结束了
    exit(0);

}
