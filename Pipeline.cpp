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
#define FILE_PATH "C:\\Users\\Mark.Wen\\Desktop\\SupremeChat\\ConsoleApplication2\\Debug\\1.txt"
#define SUCCESS 1
#define FAIL 0
#define DONE 0
#define NOT_DONE 1

class PublicCache
{
  public:
    int header;                //ǰָ���ʶ
    int end;                   //βָ���ʶ
    char cache[WIDTH][LENGTH]; //����������
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
        for (int i = 0; i < WIDTH; i++)
            memset(cache[i], '/0', LENGTH);
    }

    int judgeEmpty()
    //�жϻ������Ƿ�Ϊ��
    //���򷵻�EMPTY ���򷵻�NOT_EMPTY
    {
        if(round == 0 && header == end) return EMPTY;
        else return NOT_EMPTY;
    }

    int judgeFull()
    //�жϻ������Ƿ�Ϊ��
    //���򷵻�FULL ���򷵻�NOT_FULL
    {
       if(round == 1 && header == end) return FULL;
       else return NOT_FULL;
    }

    void readCache(char *recv)
    //��������
    //����recv ����ǰend��Ӧ�Ŀ�д��recv
    //�޸�endָ��
    //����ж��Ƿ�Ϊ��
    {
        for (int i = 0; i < LENGTH; i++)
            recv[i] = cache[end][i];
        //endָ��ָ��ǰ�ɶ��Ŀ�
        //����֮endָ��Ŀ���������Ч���ݵ�

        end = (end + 1) % WIDTH;
        if(round == 1 && end == 0) round = 0;
        //������һ��
        empty = judgeEmpty();
        full = judgeFull();
        //�޸ı�־λ
    }

    void writeCache(char *fileStr)
    //д������
    //����fileStr ����ֵд�뻺����header��Ӧ��λ�ÿ�
    //�޸�headerָ��
    //����ж��Ƿ�Ϊ��
    {
        // cout << "header: " << header << endl;
        for (int i = 0; i < LENGTH; i++)
            cache[header][i] = fileStr[i];
        //headerָ����һ����д�Ŀ�
        //����֮headerָ��Ŀ�����û����Ч���ݵ�

        if(header == WIDTH - 1) round = 1;//Խ����һ��
        header = (header + 1) % WIDTH;
        //�޸�header

        empty = judgeEmpty();
        full = judgeFull();
        //�޸ı�־λ
    }

    int getVaildSize()
    //�õ���ǰ��������Ч����Ŀ���
    {
        if(round == 0) return header - end;
        else return header + WIDTH - end;
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
    publicCache.pipeLineCond = NOT_DONE;
    //--------------------------------------------------------
    //�������ĳ�ʼ��
    //-------------------------------------------------------

    FILE* fp = fopen(FILE_PATH, "rb");
    unsigned long fileSize = 0;
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    publicCache.transTime = fileSize / LENGTH;
    publicCache.lastTimeSize = fileSize - LENGTH * publicCache.transTime;
    fclose(fp);
    //-------------------------------------------------------------------
    /* �Զ����������ļ�
        �����С
        �������鷢�ʹ��������һ�η��ʹ�С
        �޸�publicCache��Ķ�Ӧ���� */
    //--------------------------------------------------------------------

    HANDLE fTM = CreateThread(NULL, 0, fileToMem, (void*)&publicCache, 0, NULL);
    HANDLE mTN = CreateThread(NULL, 0, memToNet, (void*)&publicCache, 0, NULL);

    // Sleep(5000);
    CloseHandle(fTM);
    CloseHandle(mTN);

    while(publicCache.pipeLineCond == NOT_DONE) Sleep(20);
    //�ֶ�����

}

DWORD WINAPI fileToMem(LPVOID para)
{
	FILE *file = fopen(FILE_PATH, "rb");
    //Ҫ�����Ķ����ڴ���ļ�ָ��
    PublicCache *cachePtr = (PublicCache *)para;

    unsigned long count = 0;
    char fileStr[LENGTH] = {0};
    int writeCond = SUCCESS;

    while (count < cachePtr->transTime)
    {
        if (writeCond == SUCCESS)
        {
            fread(fileStr, LENGTH, 1, file);
            writeCond = FAIL;
			cout << "done load file " << count << endl;
        }
        //�ȶ�һ��
        //���жϻ��������ò�
        //������

        if (cachePtr->available == UNLOCK &&
            cachePtr->full == NOT_FULL)
        {
            cachePtr->available = LOCK;
            cachePtr->writeCache(fileStr);
            //��һ��д��
            if (cachePtr->getVaildSize() > 0)
                cachePtr->writeCache(fileStr);
            //�ڶ���д��
            cachePtr->available = UNLOCK;
            //һ��д�����
            writeCond = SUCCESS;
            //д�ɹ� ���Դ��ļ������һ����
            count++;
        }
        else
        {
            writeCond = FAIL;
        }
    }
    //-----------------------------------------------------
    //���鴫��
    //------------------------------------------------------
    writeCond = FAIL;
    memset(fileStr, '\0', LENGTH);
    //����
    fread(fileStr, cachePtr->lastTimeSize, 1, file);
    //���ļ�������һ������
    while (writeCond != SUCCESS)
    {
        if (cachePtr->available == UNLOCK &&
            cachePtr->full == NOT_FULL)
        {
            cachePtr->available = LOCK;
            //��head��ǰ��ָ��Ŀ�����
            //��ǿ��ɸò���
            cachePtr->writeCache(fileStr);
            cachePtr->available = UNLOCK;
            //һ��д�����
            writeCond = SUCCESS;
        }
    }
    //------------------------------------------------------
    //���һ�εĴ���
    //-------------------------------------------------------
    fclose(file);
    int a = 0;
    cin >> a;
    exit(0);
}

DWORD WINAPI memToNet(LPVOID para)
{
    PublicCache *cachePtr = (PublicCache *)para;
    unsigned long count = 0;
    char recvBuf[LENGTH] = {0};
    int readCond = SUCCESS;

    while (count < cachePtr -> transTime)
    {
        cout << "reading " << count << endl;
        if (cachePtr -> available == UNLOCK &&
            cachePtr -> empty == NOT_EMPTY)
        {
            cachePtr->available = LOCK;
            cachePtr->readCache(recvBuf);
            cout << recvBuf;
            cachePtr->available = UNLOCK;
            readCond = SUCCESS;
            count ++;
        }
        else
        {
            readCond = FAIL;
        }
    }
    //--------------------------------------------------
    //�������
    //--------------------------------------------------
    readCond = FAIL;
    memset(recvBuf, '\0', LENGTH);
    //����
    while (readCond != SUCCESS)
    {
        if (cachePtr->available == UNLOCK &&
            cachePtr->empty == NOT_EMPTY)
        {
            cachePtr->available = LOCK;
            cachePtr->readCache(recvBuf);
            cout << recvBuf;
            cachePtr->available = UNLOCK;
            readCond = SUCCESS;
        }
    }
    //--------------------------------------------------
    //���һ�εĴ���
    //-------------------------------------------------
    int a = 0;
    cin >> a;
    exit(0);
}
