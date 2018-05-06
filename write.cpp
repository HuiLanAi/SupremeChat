#include "iostream"
#include "fstream"
#include "string.h"
using namespace std;

#define SIZE 8188

int main()
{
    FILE* fp = fopen("2.jpg", "rb");
    char cache[8188] = {0};
    fseek(fp, 0, SEEK_END);
    unsigned long fileSize = ftell(fp);
    rewind(fp);
    if(fileSize > SIZE)
    {
        int transferCount = fileSize / SIZE;
        FILE* fw = fopen("3.jpg", "wb");
        for(int i = 0; i < transferCount; i ++)
        {
            fread(cache, SIZE, 1, fp);
            fwrite(cache, SIZE, 1, fw);
        }
        memset(cache, '\0', SIZE);
        int leftSize = fileSize - SIZE * transferCount;
        fread(cache, leftSize, 1, fp);
        fwrite(cache, leftSize, 1, fw);
        fclose(fw);
    }
    fclose(fp);

}
