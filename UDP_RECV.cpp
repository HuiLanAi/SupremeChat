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
// �ȳ�ʼ��һ�������
// �ڽ��յ���һ�����ĺ�����transCount
// �ڽ��յ������ڶ������ĺ�����lastTimeSize
 
//  ÿ�ν���ֱ�Ӹ��Ƶ�cache�Ķ�ά������
//  �������ö�д�� ֻҪ������header�Ϳ���*/

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
    //�����ʼ����ز���

    unsigned long transCount = 0;
    int lastTimeSize = 0;
    unsigned long curRecvCount = 0;
    //�ļ�������ز���

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\a.pdf";
    FILE* fp = fopen(path.c_str(), "wb");
    //�ļ�д��ز���

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
    //----------------���յĺ���ģ��-------------------------------------
    //-----------------------------------------------------------------
    wprintf(L"Receiving datagrams...\n");

    DWORD startTime = 0;
    DWORD endTime = 0;
    //��ʱ������
    //-------------------------------------------------------

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


    while(1)
    {
        if(curRecvCount == 0)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0, 
                                (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            startTime = GetTickCount();
            transCount = (unsigned int ) atoi(RecvBuf);
            cout << "�������Ϊ: " << transCount << endl;
            cache.transTime = transCount;
            memset(RecvBuf, '\0', 100);
            //��ʼ�������ֵ
            //���մ������
            //ע��˴���RECVBUF������
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
            //ֱ�Ӹ�ֵ��������
            //----------------------------------------------------------------
            
            cache.judgeEmpty();
            cache.judgeFull();
            //�޸Ļ�������д������
            
            curRecvCount ++;
    
        }
        //-------------------------------------------------------
        //curRecvCountΪ0ʱ �Ƚ��մ��ʹ��� �ٽ��յ�һ����
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
            //ֱ�Ӹ�ֵ��������
            //----------------------------------------------------------------

            cache.judgeEmpty();
            cache.judgeFull();
            //�޸Ļ�������д������

            // cout << curRecvCount << endl;            
            
            curRecvCount++;
        }
        //---------------------------------------------------------
        //������������
        //---------------------------------------------------------

        else if(curRecvCount == transCount)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            lastTimeSize = atoi(RecvBuf);
            cache.lastTimeSize = lastTimeSize;
            cache.available = LOCK;//��ʾlastTimeSize�Ѿ����ܵ�������ֵ
            //�������һ��Ĵ�С���޸Ļ�������Ӧ��ֵ
            //ע��˴���RECVBUF������
            //------------------------------------------------------------------

            memset(cache.cacheBuf[cache.header], '\0', CACHE_LENGTH);
            iResult = recvfrom(RecvSocket, cache.cacheBuf[cache.header], lastTimeSize, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            if(cache.header == CACHE_WID - 1) cache.round = 1;
            cache.header = (cache.header + 1) % CACHE_WID;
            cache.judgeEmpty();
            cache.judgeFull();
            //��ս����� �޸�header �޸Ļ�������д������
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
        //���պ�д�����һ��
        //---------------------------------------------------------
        else;

        if (iResult == SOCKET_ERROR)
        {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
            closesocket(RecvSocket);
        }
    }

    
    //---------------------------------------------------------------------
    //------------���Ĵ��䲿��-------------------------------------------
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

    cout << "��ʱ " << (float)(endTime - startTime) / 1000 << endl;
    cin >> path;

    

    return 0;
}



DWORD WINAPI writeFile(LPVOID para)
/* 
��ʼ�����ֲ���
�ж��ǲ���Ϊ��
��Ϊ����ֱ����������ָ��д��
�޸������� */
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
        //ֻҪ��Ϊ�վ�ʹ����д��д ÿ�ζ��궼Ҫ�޸�end
        //ѭ������������������ �޸Ŀ�����־��
        //----------------------------------------------------
    }
    //-------------------���鴫������------------------------------
    
    while(cachePtr -> judgeEmpty() == EMPTY || cachePtr -> available == UNLOCK);//�ȴ����һƬд����

    fwrite(cachePtr -> cacheBuf[cachePtr -> end], cachePtr -> lastTimeSize, 
            1, cachePtr -> fp);
    fclose(cachePtr -> fp);
    //---------------------------------------------------------------
    //���һƬ��д��
    //--------------------------------------------------------------

    cachePtr -> pipeLineCond = DONE;
    //֪ͨmain�������Խ�����
    exit(0);

}
