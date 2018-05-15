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
//�˿ںźͿռ��С�ĺ궨��

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
    //�����ļ����Ⱥͷ�Ƭ���ʹ���
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
    //����������ʼ��
    //---------------------------------------------------------
    //---------------------------------------------------------


    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    //���ڳ�ʼ��ַģ�������
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
    hints.ai_socktype = SOCK_STREAM; //STREAMģʽ
    hints.ai_protocol = IPPROTO_TCP; //TCPЭ��

    string ipAddr = "";
    cout << "������ip��ַ" << endl;
    cin >> ipAddr;
    iResult = getaddrinfo(ipAddr.c_str(), DEFAULT_PORT, &hints, &result);

    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    //�ж��Ƿ���������

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        //result�����һ����ָ��
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        //����Ϊ���ӵ���ַ������Э��ĸ���ϸ��

        //����Ϊ���ӵ�����Ķ˵�
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            //һ��ʧ�� �˴�����ٳ�������һ��
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
    //-----------------------���ͺ���ģ��--------------------------------------------
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
    //�������鴫�����
    //---------------------------------------------------------------------------
    iResult = send(ConnectSocket, to_string(cache.lastTimeSize).c_str(), 
                    to_string(cache.lastTimeSize).length(), 0);
    recv(ConnectSocket, OK, 3, MSG_WAITALL);
    //�������һƬ�Ĵ�С
    //---------------------------------------------------------------------------
    HANDLE pipeline = CreateThread(NULL, 0, writeCache,
                                   (void *)&cache, 0, NULL);
    CloseHandle(pipeline);
    //�߳����� 
    
    //------------------------------------------------------------
    unsigned long curTransCount = 0;
    while(curTransCount < transCount)
    {
        while(cache.judgeEmpty() == NOT_EMPTY && curTransCount < transCount)
        //ֻҪ�ж����ͷ������
        {
            // cout << curTransCount << endl;
            // cout << cache.header << " " << cache.end << " " << cache.round; 
            iResult = send(ConnectSocket, cache.cacheBuf[cache.end], CACHE_LENGTH, 0);
            //�ڷ��Ͷ�socket�൱�ڻ�������������
            if(cache.end == CACHE_WID - 1)
            {
                cache.round = 0;
            }
            cache.end = (cache.end + 1) % CACHE_WID;
            //�޸Ķ�д��־λ
            //------------------------------------------------------------------------
            
            curTransCount ++;
            if (iResult == SOCKET_ERROR)
            {
                printf("�ڲ�ѭ��: %d ", WSAGetLastError());
				cout << curTransCount << endl;
                Sleep(5000);
                // closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    //--------------------------------------------------------------------------
    //-------------------------���������鴫��------------------------------------
    //---------------------------------------------------------------------------

    while(cache.judgeEmpty() == EMPTY) ;
    cout << "�������Ĵ���" << endl;
    iResult = send(ConnectSocket, cache.cacheBuf[cache.end], cache.lastTimeSize, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("���һ��ʧ��: %d\n", WSAGetLastError());
        // closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
	//char OK[3] = { 0 };
	// recv(ConnectSocket, OK, 3, 0);
    cout << "���ͽ���" << endl;
    //�������һƬ
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
       //ֻҪ�пվͷ������
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
    //------------------------����������д��-----------------------------
    //-----------------------------------------------------------------
    
    memset(cachePtr->cacheBuf[cachePtr->header], '\0', CACHE_LENGTH);
    //���㵱ǰ��
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
    //���һƬ��д��
    //--------------------------------------------------------------
	return 0;
}

