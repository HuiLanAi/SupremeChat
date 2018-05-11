#include "iostream"
#include "windows.h"
#include "fstream"
using namespace std;

#define WIDTH 4
#define LENGTH 1200
#define LOCK 0
#define UNLOCK 1
#define NOT_EMPTY 0
#define EMPTY 1
#define NOT_FULL 0
#define FULL 1
#define FILE_PATH "C"

class PublicCache
{
public:
    int header;//前指针标识
    int end;//尾指针标识
    char cache[WIDTH][LENGTH];//缓冲区数组
    int available;//缓冲区读写锁
    int empty;//缓冲区是否为空的标识
    int full;//缓冲区是否满的标识
    int round;//header和end相对位置的标识
    int lastTimeSize;//最后一个文件块的大小

    void zeroSpace()
    //给数组清零
    //朱强写
    {

    }

    int judgeEmpty()
    //判断缓冲区是否为空
    //空则返回EMPTY 否则返回NOT_EMPTY
    //朱强写
    {

    }

    int judgeFull()
    //判断缓冲区是否为满
    //满则返回FULL 否则返回NOT_FULL
    //朱强写
    {

    }

    void readCache(char* recv)
    //读缓存区
    //传入recv 将当前end对应的块写入recv
    //最后修改end指针
    {

    }

    void writeCache(char* fileStr)
    //写缓存区
    //传入fileStr 将其值写入缓冲区header对应的位置块
    //最后修改header指针
    {

    }

    int getVaildSize()
    //得到当前缓冲区有效区域的块数
    {

    }



};

int main()
{
    PublicCache publicCache;
    publicCache.header = publicCache.end = publicCache.round = 0;
    

    
}

DWORD WINAPI fileToMem(LPVOID para)
{
    PublicCache* cachePtr = (PublicCache*) para;
    //---------------------------------------------
    //此处嵌入文件读写模块
    //---------------------------------------------
    while(cachePtr -> available)
    {

    }
}

