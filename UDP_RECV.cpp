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
using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 512

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

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\a.cpp";
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

    while(1)
    {
        if(curRecvCount == 0)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0, 
                                (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            transCount = (unsigned int ) atoi(RecvBuf);
            cout << "�������Ϊ: " << transCount << endl;
            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"first recvfrom failed with error %d\n", WSAGetLastError());
                closesocket(RecvSocket);
            }
            //���մ������
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            fwrite(RecvBuf, BUFLEN, 1, fp);
            //��һ����Ľ��պ�д��
            curRecvCount ++;
        }
        //-------------------------------------------------------
        //curRecvCountΪ0ʱ �Ƚ��մ��ʹ��� �ٽ��յ�һ����
        //---------------------------------------------------------

        else if(curRecvCount < transCount)
        {
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            fwrite(RecvBuf, BUFLEN, 1, fp);
            //������Ľ��պ�д��
            curRecvCount++;
        }
        //---------------------------------------------------------
        //������������
        //---------------------------------------------------------

        else if(curRecvCount == transCount)
        {
            memset(RecvBuf, '\0', BUFLEN);
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            lastTimeSize = atoi(RecvBuf);
            //�������һ��Ĵ�С
            memset(RecvBuf, '\0', BUFLEN);
            iResult = recvfrom(RecvSocket, RecvBuf, BUFLEN, 0,
                               (SOCKADDR *)&SenderAddr, &SenderAddrSize);
            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
                closesocket(RecvSocket);
            }
            fwrite(RecvBuf, lastTimeSize, 1, fp);
            fclose(fp);
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
    //-----------------------------------------------------------------
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
    return 0;
}
