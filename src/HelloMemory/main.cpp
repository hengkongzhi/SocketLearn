#include <iostream>
#include <stdlib.h>
#include <thread>
#include "../CELLTimestamp.hpp"
#include "Alloctor.h"

using namespace std;
class classA
{
public:
    classA()
    {
        printf("classA\n");
    }
    ~classA()
    {
        printf("~classA\n");
    }

};


const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;
char* data[mCount];
void workFun(int index)
{
    size_t begin = index * nCount;
    size_t end = (index + 1) * nCount;
    for (size_t i = begin; i < end; i++)
    {
        data[i] = new char [1 + (rand() % 1024)];
    }
    for (size_t i = begin; i < end; i++)
    {
        delete[] data[i];
    }

}
int main()
{
    // thread t[tCount];
    // for (int n = 0; n < tCount; n++)
    // {
    //     t[n] = thread(workFun, n);
    // }
    // CELLTimestamp tTime;
    // for (int n = 0; n < tCount; n++)
    // {
    //     t[n].join();
    // }
    // cout << tTime.getElapsedTimeInMilliSec() << ", main thread" << endl;
    int* a = new int;
    *a = 100;
    printf("%d\n", *a);
    delete a;
    shared_ptr<int> b = make_shared<int>();
    *b = 100;
    printf("%d\n", *b);
    classA* a1 = new classA();
    delete a1;
    shared_ptr<classA> b1 = make_shared<classA>();
    return 0;
}