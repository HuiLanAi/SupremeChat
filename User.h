#ifndef USER_H  
#define USER_H  

#include "vector"
#include "map"
#include<iostream>
#include<string.h>
using namespace std;
#include <fstream>
#define LOGIN_FLAG '1'
#define FRIMES_FLAG '2'
#define INQU_FLAG '3'



//消息
class Message
{
    public:
    // 0|12|12|12121|i love you
	
   Message(string s )
    {  
        for(int i=0;i<30;i++)
        launchTime[i]=0;
        int i=0;
        flag=s[i++];
        //1
        i++;
        string temp="";
        while(s[i]!='|')
        {
            temp=+s[i];
            i++;
        }
        sender=atoi(temp.c_str());
       
       //2.
        i++;
        temp="";
        while(s[i]!='|')
        {
            temp=+s[i];
            i++;
        }
        receiver=atoi(temp.c_str());
        //3.
        i++;
        temp="";
        while(s[i]!='|')
        {
            temp=temp+s[i];
            i++;
        }
        
        int k=0;
        while (temp[k] != 0)
        {
            launchTime[k] = temp[k];
            k++;
        }

        //4.
        i++;
        message="";
        while(s[i]!='\0')
        {
            message+=s[i++];
        }
    }
    
    char flag;
    int sender;
    int receiver;
    char launchTime[30];
    string message;
 
};

//缓冲区
void  storeMessage(map<int,Message>*cache , string s )
{
    Message m(s);
    cache->insert( pair<int,Message>(m.sender,m) );
    cache->insert( pair<int, Message>(m.receiver,m));
}

class User
{
public:
    int id;
	char cipher[7];
    bool onlineFlag;
    vector<User> friend;
    string IPaddress;    
    vector<Message> historyMessage;
    void Cut(vector<int>& v , char*p);//分割函数
};



void initialize(vector<User>* u )
{
    for(int i=0;i<3;i++)
    {
       User u1 ;
       u1.IPaddress="";
       u1.id=i;
       for(int j=0;j<6;j++)
       {
           u1.cipher[j]=i+'0';
       }
       u1.cipher[6]=0;

       u1.onlineFlag=false;
     u->push_back(u1);
    }
}

#endif 