/* 数据库初始化函数 */
#include <vector>
#include <User.h>
using namespace std;



void initialize(vector<User>*u )
{
    for(int i=0;i<3;i++)
    {
       u[i].IPaddress="";
       u[i].id=i;
       for(int j=0;j<6;j++)
       {
           u[i].cipher[j]=i;
       }
       onLineFlag=false;
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