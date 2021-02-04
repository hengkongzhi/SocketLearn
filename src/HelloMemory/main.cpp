#include <iostream>
#include <stdlib.h>
#include <thread>
#include "../CELLTimestamp.hpp"
#include "Alloctor.h"

using namespace std;
class classA
{
public:
    classA(int a)
    {
        num = a;
        printf("classA\n");
    }
    ~classA()
    {
        printf("~classA\n");
    }
public:
    int num = 0;

};
shared_ptr<classA> fun(shared_ptr<classA> pA)
{
    pA->num++;
    shared_ptr<classA> pB = pA;
    return pB;
}
classA* fun(classA* pA)
{
    pA->num++;
    classA* pB = pA;
    return pB;
}

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
    // int* a = new int;
    // *a = 100;
    // printf("%d\n", *a);
    // delete a;
    // shared_ptr<int> b = make_shared<int>();
    // *b = 100;
    // printf("%d\n", *b);
    // classA* a1 = new classA();
    // delete a1;
    {
        shared_ptr<classA> b = make_shared<classA>(100);
        CELLTimestamp tTime;
        for (int i = 0; i < 10000000; i++)
        {
            shared_ptr<classA> c = fun(b);
        }
        cout << tTime.getElapsedTimeInMilliSec() << endl;
    }

    {
        classA* b = new classA(100);
        CELLTimestamp tTime;
        for (int i = 0; i < 10000000; i++)
        {
            classA* c = fun(b);
        }
        cout << tTime.getElapsedTimeInMilliSec() << endl;
        delete b;
    }

    return 0;
}