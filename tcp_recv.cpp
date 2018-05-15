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
//�˿ںźͿռ��С�ĺ궨��

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
//���߳̿������ĺ궨��


class Cache
{
  public:
    FILE* fp;                  //�ļ�ָ��
    int header;                //ǰָ���ʶ
    int end;                   //βָ���ʶ
    char cacheBuf[CACHE_WID][CACHE_LENGTH]; //����������
    int available;             //��������д��
    int empty;                 //�������Ƿ�Ϊ�յı�ʶ
    int full;                  //�������Ƿ����ı�ʶ
    int round;                 //header��end���λ�õı�ʶ
    int lastTimeSize;          //���һ���ļ���Ĵ�С
    unsigned long transTime;   //�ļ����鴫��Ĵ���
    int pipeLineCond;

    void zeroSpace()
    //����������
    {
        for (int i = 0; i < CACHE_WID; i++)
            memset(cacheBuf[i], '/0', CACHE_LENGTH);
    }

    int judgeEmpty()
    //�жϻ������Ƿ�Ϊ��
    //���򷵻�EMPTY ���򷵻�NOT_EMPTY
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
    //�жϻ������Ƿ�Ϊ��
    //���򷵻�FULL ���򷵻�NOT_FULL
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
    //�ļ�������ز���

    Cache cache;
    cache.fp = fp;
    cache.header = cache.end = cache.round = 0;
    cache.empty = EMPTY;
    cache.full = NOT_FULL;
    cache.available = UNLOCK;
    cache.zeroSpace();
    cache.pipeLineCond = NOT_DONE;
    //����������ʼ��
    //---------------------------------------------------------

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    //��Socket�汾��������
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    //������
    hints.ai_family = AF_INET;//�޶�Ϊipv4
    hints.ai_socktype = SOCK_STREAM;//�޶�Ϊstreamģʽ����
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;//��¼�������ӵ�ip��ַ

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    //�ѷ������˿ڡ���ַ�����ò���д��result hints�൱��һ���н�
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //��ListenSocket�����ճ�ʼ����socket�ĸ�����Ϣ
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket

    //����bind()
    //һ��socket�������е�ַ�� ������ַ �˿ں����
    //socket����ָ��һ����ַ�� bind����ָ����ַ�Ͷ˿ں�
    //���bind����ֻ�����ڻ�û�����ӵ�socket ֻ����connect()��listen()֮ǰ����
    //һ��socketֻ�ܵ���һ��bind
    //������һ��bind֮��socket�Ͳ����ٸ�����
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
    //ͨ���������ֻ�д��ѭ�� ֱ�������ӽ���Ϊֹ
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    //�������ӷ���ʱ ����accept �����ǵ��̵߳�
    //һ˵�����ӷ���accept�����ú� ����������Ὺһ�����߳������������
    //������������ż���˳��ִ�л��߼���������������
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
    //-------------------------���պ��Ĳ���---------------------------------------------------------
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
    //���շ��ʹ�������ʼ����Ϣ
    //----------------------------------------------------------------
    memset(countStr, '\0', 30);
    iResult = recv(ClientSocket, countStr, 30, 0);
    lastTimeSize = cache.lastTimeSize = (int)atoi(countStr);
    //�������һ����ĳߴ�
    //----------------------------------------------------------------

    HANDLE pipeline = CreateThread(NULL, 0, readCache,
                                   (void *)&cache, 0, NULL);
    CloseHandle(pipeline);
    cout << "�ļ�������� " << transCount << endl;
    cout << endl << "LTS: " << cache.lastTimeSize << endl;
    Sleep(10000);
    //�߳����� 

    while(curRecvCount < transCount)
    {
        while(cache.judgeFull() == NOT_FULL && curRecvCount < transCount)
        //ֻҪ����λ�ͷ������
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
            //�������
            //�ļ�����
            //�Ķ�д���λ
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
    //����������
    //----------------------------------------------------------------

    memset(cache.cacheBuf[cache.header], '\0', CACHE_LENGTH);
    while(cache.judgeFull() == FULL);
    //������վ�һֱ������
    iResult = recv(ClientSocket, cache.cacheBuf[cache.header], 
                    lastTimeSize, 0);
    if (cache.header == CACHE_WID - 1)
    {
        cache.round = 1;
        cache.header = 0;
    }
    else
        cache.header++;
    //���һ�ν���
    //�ļ�����
    //�Ķ�д���λ
    //socket��ʱ�������� ���Ķ�д���λ�ǲ����ܵ�
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
        //�ʵ�����д���ļ����ٶ� Ϊ���ն��ṩ����ʱ�� ÿ�ζ��궼Ҫ�޸�end
        //ѭ������������������ �޸Ŀ�����־��
        //----------------------------------------------------
    }
    //-------------------���鴫������------------------------------
    
    while(cachePtr -> judgeEmpty() == EMPTY) ;//�ȴ����һƬд����
    fwrite(cachePtr->cacheBuf[cachePtr->end], cachePtr->lastTimeSize,
           1, cachePtr->fp);
    fclose(cachePtr->fp);
    cachePtr->pipeLineCond = DONE;
    cout << "done save" << endl;
    return 0;
}