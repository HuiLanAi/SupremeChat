#include "iostream"
#include "windows.h"
#include "fstream"
#include "cstring"
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
#define  SUCCESS 1
#define FAIL 0

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
    unsigned long transTime;//文件整块传输的次数

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
    //修改end指针
    //最后判断是否为空
    {

    }

    void writeCache(char* fileStr)
    //写缓存区
    //传入fileStr 将其值写入缓冲区header对应的位置块
    //修改header指针
    //最后判断是否为满
    {

    }

    int getVaildSize()
    //得到当前缓冲区有效区域的块数
    {

    }



};

DWORD WINAPI fileToMem(LPVOID para);
DWORD WINAPI memToNet(LPVOID para);


int main()
{
    PublicCache publicCache;
    publicCache.header = publicCache.end = publicCache.round = 0;
    publicCache.empty = EMPTY;
    publicCache.full = NOT_FULL;
    publicCache.available = UNLOCK;
    publicCache.zeroSpace();
    //--------------------------------------------------------
    //缓冲区的初始化
    //-------------------------------------------------------

    /* 以二进制流打开文件
        计算大小
        计算整块发送次数和最后一次发送大小
        修改publicCache里的对应属性 */
    //上述功能朱强完成

    
}

DWORD WINAPI fileToMem(LPVOID para)
{
    FILE* file;//要操作的读入内存的文件指针
    //朱强在此处修改 让file以二进制流打开FILE_PATH的文件
    
    PublicCache* cachePtr = (PublicCache*) para;

    unsigned long count = 0;
    char fileStr[LENGTH] = { 0 };
    int writeCond = SUCCESS;

    while(count < cachePtr -> transTime)
    {
        if(writeCond == SUCCESS)
            fread(fileStr, LENGTH, 1, file);
        //先读一个
        //再判断缓冲区可用不
        //再上锁
        
        if (cachePtr -> available == UNLOCK && 
            cachePtr -> full == NOT_FULL)
        {
            cachePtr -> available = LOCK;
            cachePtr -> writeCache(fileStr);
            //第一次写入
            if(cachePtr -> getVaildSize() > 0)
                cachePtr -> writeCache(fileStr);
            //第二次写入
            cachePtr -> available = UNLOCK;
            //一个写入完成
            writeCond = SUCCESS;
            //写成功 可以从文件里读下一个块
            count ++;
        }
        else
        {
            writeCond = FAIL;
        }
    }
    //-----------------------------------------------------
    //整块传送
    //------------------------------------------------------
    writeCond = FAIL;
    memset(fileStr, '\0', LENGTH);
    //清零
    fread(fileStr, cachePtr -> lastTimeSize, 1, file);
    //从文件里读最后一个部分
    while(writeCond != SUCCESS)
    {
        if (cachePtr->available == UNLOCK &&
            cachePtr->full == NOT_FULL)
        {
            cachePtr -> available = LOCK;
            //对head当前所指向的块清零
            //朱强完成该部分
            cachePtr -> writeCache(fileStr);
            cachePtr -> available = UNLOCK;
            //一个写入完成
            writeCond = SUCCESS;
        }    
    }
    //------------------------------------------------------
    //最后一次的传送
    //-------------------------------------------------------
    fclose(file);
    exit(0);
}

DWORD WINAPI memToNet(LPVOID para)
{
    PublicCache* cachePtr = (PublicCache*) para;
    unsigned long count = 0;
    char recvBuf[LENGTH] = { 0 };
    int readCond = SUCCESS;

    while(count < cachePtr -> transTime)
    {
        if(cachePtr -> available == UNLOCK &&
            cachePtr -> empty == NOT_EMPTY)
        {
            cachePtr -> available = LOCK;
            cachePtr -> readCache(recvBuf);
            cout << recvBuf ;
            cachePtr -> available = UNLOCK;
            readCond = SUCCESS;
            count ++;
        }
        else
        {
            readCond = FAIL;
        }
    }
    //--------------------------------------------------
    //整块接收
    //--------------------------------------------------
    readCond = FAIL;
    memset(recvBuf, '\0', LENGTH);
    //清零
    while(writeCond != SUCCESS)
    {
        if(cachePtr -> available == UNLOCK &&
            cachePtr -> empty == NOT_EMPTY)
        {
            cachePtr -> available = LOCK;
            cacheptr -> readCache(recvBuf);
            cout << recvBuf;
            cachePtr -> available = UNLOCK; 
            readCond = SUCCESS;
        }
    }
    //--------------------------------------------------
    //最后一次的传送
    //-------------------------------------------------
    exit(0);
}

