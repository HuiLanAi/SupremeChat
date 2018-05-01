#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "string"
#include "iostream"
using namespace std;

#define LOGIN_FLAG '1'
#define FRIMES_FLAG '2'


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int sentMessage(SOCKET* connectSocket, char* sendMes, int mesLength, char flag)
{
    if(flag == LOGIN_FLAG)
    {
        string temp = "1";
        temp.append("|");
        temp.append(sendMes);
        for(int i = 0; i < 512; i ++)
            sendMes[i] = 0;
        int result = send(*connectSocket, temp.c_str(), temp.length(), 0);
        
    }
    
    else if (flag == FRIMES_FLAG)
    {

    }

    else return 0;
}