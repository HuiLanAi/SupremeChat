#include<iostream>
using namespace std;
#include<stdlib.h>
#include<string.h>
#include<vector>
class User
{
public:
    User()
    {

    };
    int id;
    char cipher[7];
    bool onlineFlag;
    vector<User> friend;
    string IPaddress;
};

void initialize(vector<User>&u )
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
       u.push_back(u1);
    }

 for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)//除了自己均是好友
        {
            if(j!=i)
            {
                u[i].friend.push_back( u[j]  );
            }
        }
    }
}
int checkUser(vector<User> &user, string s)
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
    char inCode[6];
	for (int i = 0;i < 6;i++)
		inCode[i] = temp[i];
		inCode[6]=0;
		cout<<"code in the database:"<<inCode<<endl;
    //IP
    string ip = "";


    while (s[i] != 0)
    {
        ip += s[i];
        i++;
    }


    //check
    int ok = 0;
    for (int i = 0; i < user.size(); i++)
    {
        if (user[i].id == inID && strcmp(user[i].cipher, inCode) != 0)
        {
            return (-1); //pass_word错误
        }
        else if (user[i].id == inID && strcmp(user[i].cipher, inCode) == 0)
        {
           user[i].IPaddress= ip;
		   user[i].onlineFlag=true;
           return (0); //验证成功
        }
    }
    return (-2); //不存在该用户
}
int main()
{
    vector<User>u;
    initialize(u);
    string s="11|111110|147.21.21.21";
    cout<<checkUser(u, s)<<endl;
}
