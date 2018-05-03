/*服务器端后台处理函数库
不含任何带Socket的功能 */

#pragma once
#include "User.h"
#include "string.h"
#include "string"
#include "windows.h"
#define WIN32_LEAN_AND_MEAN
#define INQUIRY_PORT "27016"
#define DEFAULT_BUFLEN 512
#define INQ_FLAG 3


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

using namespace std;

class MesCacheInfo
{
	public:
	int size;
	//报文缓存块的大小
	vector<Message>* mesCache;
	//报文缓存块的地址

	MesCacheInfo(int i, vector<Message>* m)
	{
		size = i;
		mesCache = m;
	}

};


int check_user(vector<User>& user, string s);
string checkMesCache (MesCacheInfo* mesCacheInfo, int userid);

string OutOfNetwork(string rualMessage, vector<User>& dataBase, 
                    MesCacheInfo* mesCacheInfo)
//退出要想好用那种报文头合适
{
	vector<Message>* mesCache= mesCacheInfo -> mesCache;
	int k=1;
    string temp="";
	while(rualMessage[k]!='|')
	{ 
	  temp=temp+rualMessage[k++];

	}	

	cout << temp << " user" << endl;

	int userid = atoi(temp.c_str());

	if(rualMessage[0] == LOGIN_FLAG) //登录验证
    {
		int checkRes = check_user(dataBase, rualMessage);
        if(checkRes == 0)//身份匹配成功
        {
            string retVal = to_string(userid);
			cout << retVal << "用户 LOGIN" << endl;
			retVal = '0';
			
            return retVal;
        }
        else if(checkRes == -1)
        {
            string retVal = "1";
            return retVal;
        }
        else
		{
			string retVal = "2";
            return retVal;
        }
    }

	else if(rualMessage[0] == INQ_FLAG)
	{
		//登录部分要改成配合多线程的
		int inquiryOrigin = 0;
		string temp = "";
		//嗅探来源用户名
		for(int i = 1; i < rualMessage.size(); i ++)
			temp += rualMessage[i];
		inquiryOrigin = atoi(temp.c_str());
		string recvMessage = checkMesCache(mesCacheInfo, inquiryOrigin);
		return recvMessage;
	}

    else if (rualMessage[0] == FRIMES_FLAG)//普通报文
	// 仅仅写入就行了
    {
		Message curMesg = Message(rualMessage); //当前信息
		(*mesCache).push_back(curMesg);			//收入缓存池
		mesCacheInfo -> size ++;
		//封装类中要做到写入时的大小同步更新

		// string idChar = rualMessage;
		// int posi = idChar.find('|');
		// idChar = idChar.substr(posi, idChar.length() - posi- 1);
		//得到接受者id的字符串
		
		// string idChar = rualMessage;
		// int posi = idChar.find('|');
		// idChar = idChar.substr(0, posi - 1);
		// int userid = atoi(idChar.c_str());

		// string retVal = checkMesCache(mesCache, userid);
		// cout << retVal << " FRI_MES" << endl;
		string retVal = "";
		return retVal;
		//返回一个空字符串
	}
}

int check_user(vector<User>& user, string s)
//用户身份验证函数
{
	int i = 0;
	i++; //i=1
	string temp = "";
	//1ID|code|ip
	while (s[i] != '|')
	{
		temp += s[i];
		i++;
	}
	int inID = atoi(temp.c_str());
	//PASS_WORD
	i++;
	temp = "";
	while (s[i] != '|')
	{
		temp += s[i];
		i++;
	}
	char inCode[7];
	for (int i = 0;i < 6;i++)
		inCode[i] = temp[i];
	inCode[6] = 0;
	cout << "code in the database:" << inCode << endl;
	//IP
	string ip = "";


	while (s[i] != 0)
	{
		ip += s[i];
		i++;
	}


	//check
	int ok = 0;
	for (int i = 0; i < (user).size(); i++)
	{
		if ((user)[i].id == inID && strcmp((user)[i].cipher, inCode) != 0)
		{
			return (-1); //pass_word错误
		}
		else if ((user)[i].id == inID && strcmp((user)[i].cipher, inCode) == 0)
		{
			(user)[i].IPaddress = ip;
			(user)[i].onlineFlag = true;
			return (0); //验证成功
		}
	}
	return (-2); //不存在该用户
}


string checkMesCache (MesCacheInfo* mesCacheInfo, int userid)
//检查是否有改用户的消息缓存
//格式为：时间 发送者 换行 消息 换行
//每提取出一条消息 就将该消息在数据库中删除
{
    string retVal = "*";
	if(mesCacheInfo -> size == 0) return retVal;

	vector<Message>* mesCache = mesCacheInfo -> mesCache;

    for(int i = 0; i < (mesCacheInfo -> size); i ++)
    {
        if((*mesCache).at(i).receiver == userid)
        {
            retVal += (*mesCache). at(i).launchTime;
            retVal += " ";
            retVal += to_string((*mesCache). at(i).sender) + "\n"
						+ (*mesCache). at(i).message + "\n";
			vector<Message>:: iterator cur = ((*mesCache).begin());
			(*mesCache).erase(cur + i);
			mesCacheInfo -> size --;
			//封装类中删除一条消息时大小也要同步更新
			i -= 1;
        }
    }
	cout << retVal << " check" << endl;
	return retVal;
}


DWORD WINAPI handleInquiry(LPVOID mesCacheInfo)
//服务器端的监听线程
//传入的参数只要消息数据库就够了
// bug在于mesCache作为一个空指针传进来的时候 其size属性就被改乱掉了 之后会溢出界限
// 解决方案：打算新建一个类 把mesCache和size封装在一起
// 但是首先 1. 要改动checkMesCache
// 2. 当有消息传入或者被删除时 如何保证size的值的实时更新呢

{
	// vector<Message>& tempMesCache = (vector<Message>&) mesCache;

    WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	char recvBuf[DEFAULT_BUFLEN] = {0};

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//对Socket版本进行限制
	if (iResult != 0) {
		printf("子线程Socket版本故障: %d\n", iResult);
		return 1;
	}

    cout << "子线程Socket版本初始化完毕" << endl;

	ZeroMemory(&hints, sizeof(hints));
	//先清零
	hints.ai_family = AF_INET;//限定为ipv4
	hints.ai_socktype = SOCK_STREAM;//限定为stream模式传送
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;//记录进入连接的ip地址

    iResult = getaddrinfo(NULL, INQUIRY_PORT, &hints, &result);
	//把服务器端口、地址和配置参数写入result hints相当于一个中介
	//此处的端口改为27016
	if (iResult != 0) {
		printf("子线程参数初始化故障: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	cout << "子线程Socket参数初始化完毕" << endl;


    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//用ListenSocket来接收初始配置socket的各种信息
	if (ListenSocket == INVALID_SOCKET) {
		printf("子线程监听初始化故障: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	cout << "子线程Socket监听初始化完毕" << endl;

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	//绑定
	if (iResult == SOCKET_ERROR) {
		printf("子线程绑定故障: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	cout << "子线程Socket绑定初始化完毕" << endl;

	while(true)
    {
        iResult = listen(ListenSocket, SOMAXCONN);

        //通常监听部分会写成循环 直到有连接接入为止
        if (iResult == SOCKET_ERROR)
        {
            printf("子线程监听失败: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        //循环监听

        cout << "子线程正在等待接入..." << endl;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
		{
			cout << "子线程接收失败" << endl;
			continue;
		}
		cout << "子线程收到一个消息: " << endl;

        if (ClientSocket == INVALID_SOCKET)
        {
            printf("子线程接收失败: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            // WSACleanup();
            return 1;
        }

        for (int i = 0; i < DEFAULT_BUFLEN; i++)
			recvBuf[i] = 0;
        //在接收报文前清空接收池

		iResult = recv(ClientSocket, recvBuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("%s\n", recvBuf);
            string recvBufToStr = recvBuf;
            string retVal = " ";
			// recvBufToStr是转成string的报文

			string temp = "";
			for (int i = 1; i < recvBufToStr.size(); i++)
				temp += recvBufToStr[i];
			int userid = atoi(temp.c_str());
			//嗅探来源用户名

			retVal = checkMesCache((MesCacheInfo*)mesCacheInfo, userid);

            cout << "子线程发送" << endl << retVal << endl;

            int res = send(ClientSocket, retVal.c_str(), retVal.length(), 0);
            // 马上要修改 要适应多线程的修改 此处只返回发送成功的信号就行了 要改动OutOfNetwork函数
            if (res == SOCKET_ERROR)
            {
                printf("发送失败: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)
            printf("连接即将关闭");
        else
        {
            printf("连接发生错误", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        // closesocket(ClientSocket);
    }
}
