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

#define BUFLEN 400
#define WINDOW_SIZE 9

// class SPP_SEND_CTRL
// {
//     char window [WINDOW_SIZE][BUFLEN];
//     int end;
//     int head;
//     int missNo;
// };







int main()
{
    string hostIpAddr = "";
    cout << "输入主机ip 地址" << endl;
    cin >> hostIpAddr;
    char SendBuf[BUFLEN] = {0};//缓冲区

    string path = "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\aaa.pdf";
    unsigned long fileSize = 0;
    FILE* fp = fopen(path.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    // char* fileInMem = new char [fileSize];
    // memset(fileInMem, 0, fileSize);
    unsigned long transCount = fileSize / BUFLEN;
    unsigned long curSendTime = 0;
    int lastTimeSize = fileSize - BUFLEN * transCount;
    //------------------------------------------------------------
    //计算文件长度和分片发送次数
    //------------------------------------------------------------


    int iResult;
    WSADATA wsaData;

    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;

    unsigned short Port = 27015;


    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //---------------------------------------------
    // Create a socket for sending data
// #define DEFAULT_BUFLEN 512
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr(hostIpAddr.c_str());

    //---------------------------------------------
    // Send a datagram to the receiver
    wprintf(L"Sending a datagram to the receiver...\n");
    //--------------------------------------------------------
    //---------------------------------------------------------
    //---------------------------------------------------------
    //发送的核心模块
    while(1)
    {
        if(curSendTime == 0)
        {
            iResult = sendto(SendSocket, to_string(transCount).c_str(), 
                            to_string(transCount).length(), 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"first time sendto failed with error: %d\n", WSAGetLastError());
                closesocket(SendSocket);
                WSACleanup();
                return 1;
            }
            //第一次发送传送次数
            fread(SendBuf, BUFLEN, 1, fp);
            iResult = sendto(SendSocket, SendBuf,
                             BUFLEN, 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
            curSendTime ++;
            //发送一个整块
        }
        //--------------------------------------------------------------------------------
        //cursendTime为0时 先传输总发送次数 再传送第一个块
        //-----------------------------------------------------------------------------

        else if(curSendTime < transCount)
        {
			// if (curSendTime % 32 == 0 && curSendTime != 0) Sleep(80);
            // if (curSendTime % 100 == 0 && curSendTime != 0) Sleep(80);
            cout << curSendTime << endl;
            fread(SendBuf, BUFLEN, 1, fp);
            iResult = sendto(SendSocket, SendBuf,
                             BUFLEN, 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
            curSendTime++;
            Sleep(2);
			if (curSendTime % 100 == 0) Sleep(100);
        }
        //--------------------------------------------------------------------------
        //连续传送整块
        //--------------------------------------------------------------------------

        else if(curSendTime == transCount) 
        {
            memset(SendBuf, '\0', BUFLEN);
            iResult = sendto(SendSocket, to_string(lastTimeSize).c_str(),
                             to_string(lastTimeSize).length(), 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
                closesocket(SendSocket);
                WSACleanup();
                return 1;
            }
            //-----------------------------------------------------------------------
            //发送最后一个块的大小
            //-----------------------------------------------------------------------
            memset(SendBuf, '\0', BUFLEN);
            fread(SendBuf, lastTimeSize, 1, fp);
            iResult = sendto(SendSocket, SendBuf,
                             lastTimeSize, 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
			Sleep(5);
            if (iResult == SOCKET_ERROR)
            {
                wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
                closesocket(SendSocket);
                WSACleanup();
                return 1;
            }
            fclose(fp);
            //--------------------------------------------------------------------
            //读和发送最后一个块
            //----------------------------------------------------------------------
            
            break;
        }
        else ;
        //---------------------------------------------------------------------------------
        //全部的发送流程
        //------------------------------------------------------------------------------
        if (iResult == SOCKET_ERROR)
        {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            closesocket(SendSocket);
            WSACleanup();
            return 1;
        }
        //标准检查式结尾
    }

    Sleep(5000);
    //---------------------------------------------------------
    //---------------------------------------------------------
    //---------------------------------------------------------

    //---------------------------------------------
    // When the application is finished sending, close the socket.
    wprintf(L"Finished sending. Closing socket.\n");
    iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Clean up and quit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    return 0;
}

// DWORD WINAPI SPP_CheckRecv(LPVOID para)
// {

// }