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
#define CACHE_WID 42
#define CACHE_LENGTH DEFAULT_BUFLEN
#define DEFAULT_PORT "27015"
//端口号和空间大小的宏定义

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

DWORD WINAPI writeCache(LPVOID para);


int __cdecl main()
{

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\aaa.mp4";
    unsigned long fileSize = 0;
    FILE* fp = fopen(path.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    unsigned long transCount = fileSize / DEFAULT_BUFLEN;
    int lastTimeSize = fileSize - DEFAULT_BUFLEN * transCount;
    cout << transCount << endl;
    cout << lastTimeSize << endl;
    //------------------------------------------------------------
    //计算文件长度和分片发送次数
    //------------------------------------------------------------
    Cache cache;
    cache.fp = fp;
    cache.header = cache.end = cache.round = 0;
    cache.empty = EMPTY;
    cache.full = NOT_FULL;
    cache.available = UNLOCK;
    cache.zeroSpace();
    cache.pipeLineCond = NOT_DONE;
    cache.lastTimeSize = lastTimeSize;
    cache.transTime = transCount;
    //类对象参数初始化
    //---------------------------------------------------------
    //---------------------------------------------------------


    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    //关于初始地址模块的声明
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; //STREAM模式
    hints.ai_protocol = IPPROTO_TCP; //TCP协议

    string ipAddr = "";
    cout << "服务器ip地址" << endl;
    cin >> ipAddr;
    iResult = getaddrinfo(ipAddr.c_str(), DEFAULT_PORT, &hints, &result);

    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    //判断是否能连得上

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        //result最初是一个空指针
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        //上述为连接到地址、配置协议的各种细节

        //下面为连接到具体的端点
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            //一旦失败 此处最好再尝试连接一次
            // // closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    //------------------------------------------------------------------------------
    //-----------------------发送核心模块--------------------------------------------
    //-----------------------------------------------------------------------------
    char OK[3] = {0};
    iResult = send(ConnectSocket, to_string(transCount).c_str(), 
                    to_string(transCount).length(), 0);
    recv(ConnectSocket, OK, 3, MSG_WAITALL);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        Sleep(5000);
        // closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    //发送整块传输次数
    //---------------------------------------------------------------------------
    iResult = send(ConnectSocket, to_string(cache.lastTimeSize).c_str(), 
                    to_string(cache.lastTimeSize).length(), 0);
    recv(ConnectSocket, OK, 3, MSG_WAITALL);
    //发送最后一片的大小
    //---------------------------------------------------------------------------
    HANDLE pipeline = CreateThread(NULL, 0, writeCache,
                                   (void *)&cache, 0, NULL);
    CloseHandle(pipeline);
    //线程启动 
    
    //------------------------------------------------------------
    unsigned long curTransCount = 0;
    while(curTransCount < transCount)
    {
        while(cache.judgeEmpty() == NOT_EMPTY && curTransCount < transCount)
        //只要有东西就疯狂的输出
        {
            // cout << curTransCount << endl;
            // cout << cache.header << " " << cache.end << " " << cache.round; 
            iResult = send(ConnectSocket, cache.cacheBuf[cache.end], CACHE_LENGTH, 0);
            //在发送端socket相当于缓冲区的消费者
            if(cache.end == CACHE_WID - 1)
            {
                cache.round = 0;
            }
            cache.end = (cache.end + 1) % CACHE_WID;
            //修改读写标志位
            //------------------------------------------------------------------------
            
            curTransCount ++;
            if (iResult == SOCKET_ERROR)
            {
                printf("内部循环: %d ", WSAGetLastError());
				cout << curTransCount << endl;
                Sleep(5000);
                // closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    //--------------------------------------------------------------------------
    //-------------------------规整的整块传送------------------------------------
    //---------------------------------------------------------------------------

    while(cache.judgeEmpty() == EMPTY) ;
    cout << "开启最后的传送" << endl;
    iResult = send(ConnectSocket, cache.cacheBuf[cache.end], cache.lastTimeSize, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("最后一次失败: %d\n", WSAGetLastError());
        // closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
	//char OK[3] = { 0 };
	// recv(ConnectSocket, OK, 3, 0);
    cout << "发送结束" << endl;
    //发送最后一片
    //------------------------------------------------------------------

    //----------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------
    
    
    iResult = shutdown(ConnectSocket, SD_SEND);
    // cleanup
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        // closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    // // closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}



DWORD WINAPI writeCache(LPVOID para)
{
    Cache* cachePtr = (Cache*) para;
    unsigned long count = 0;

    while(count < cachePtr -> transTime)
    {
       while(cachePtr->judgeFull() == NOT_FULL && count < cachePtr -> transTime)
       //只要有空就疯狂的输出
       {
           fread(cachePtr->cacheBuf[cachePtr->header],
                 CACHE_LENGTH, 1, cachePtr->fp);
           if (cachePtr->header == CACHE_WID - 1)
           {
               cachePtr->round = 1;
               cachePtr->header = 0;
           }
           else
               cachePtr->header++;
           count++;
       }

    }   
    //----------------------------------------------------------------
    //------------------------规整的整块写入-----------------------------
    //-----------------------------------------------------------------
    
    memset(cachePtr->cacheBuf[cachePtr->header], '\0', CACHE_LENGTH);
    //清零当前块
    fread(cachePtr->cacheBuf[cachePtr->header], cachePtr->lastTimeSize,
           1, cachePtr->fp);
    fclose(cachePtr->fp);
    if (cachePtr->header == CACHE_WID - 1)
    {
        cachePtr->round = 1;
        cachePtr->header = 0;
    }
    else
        cachePtr->header++;
    cout << "done write" << endl;
    //---------------------------------------------------------------
    //最后一片的写入
    //--------------------------------------------------------------
	return 0;
}

