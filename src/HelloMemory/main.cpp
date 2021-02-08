#include <iostream>
#include <stdlib.h>
#include <thread>
#include "../CELLTimestamp.hpp"
#include "Alloctor.h"
#include "CELLObjectPool.hpp"

using namespace std;
class classA:public ObjectPoolBase<classA, 6>
{
public:
    classA(int a)
    {
        num = a;
        // printf("classA\n");
    }
    ~classA()
    {
        // printf("~classA\n");
    }
public:
    int num = 0;

};
class classB:public ObjectPoolBase<classB, 10>
{
public:
    classB(int a, int b)
    {
        num = a * b;
        printf("classB\n");
    }
    ~classB()
    {
        printf("~classB\n");
    }
public:
    int num = 0;

};
// shared_ptr<classA> fun(shared_ptr<classA> pA)
// {
//     pA->num++;
//     shared_ptr<classA> pB = pA;
//     return pB;
// }
// classA* fun(classA* pA)
// {
//     pA->num++;
//     classA* pB = pA;
//     return pB;
// }

const int tCount = 4;
const int mCount = 28;
const int nCount = mCount / tCount;
classA* data[mCount];
void workFun(int index)
{
    size_t begin = index * nCount;
    size_t end = (index + 1) * nCount;
    for (size_t i = begin; i < end; i++)
    {
        data[i] = classA::createObject(6);
        // data[i] = new char [1 + (rand() % 1024)];
    }
    for (size_t i = begin; i < end; i++)
    {
        classA::destroyObject(data[i]);
        // delete[] data[i];
    }

}
int main()
{
    thread t[tCount];
    for (int n = 0; n < tCount; n++)
    {
        t[n] = thread(workFun, n);
    }
    CELLTimestamp tTime;
    for (int n = 0; n < tCount; n++)
    {
        t[n].join();
    }
    cout << tTime.getElapsedTimeInMilliSec() << ", main thread" << endl;
    // int* a = new int;
    // *a = 100;
    // printf("%d\n", *a);
    // delete a;
    // shared_ptr<int> b = make_shared<int>();
    // *b = 100;
    // printf("%d\n", *b);
    // classA* a1 = new classA();
    // delete a1;
    // {
    //     shared_ptr<classA> b = make_shared<classA>(100);
    //     CELLTimestamp tTime;
    //     for (int i = 0; i < 10000000; i++)
    //     {
    //         shared_ptr<classA> c = fun(b);
    //     }
    //     cout << tTime.getElapsedTimeInMilliSec() << endl;
    // }

    // {
    //     classA* b = new classA(100);
    //     CELLTimestamp tTime;
    //     for (int i = 0; i < 10000000; i++)
    //     {
    //         classA* c = fun(b);
    //     }
    //     cout << tTime.getElapsedTimeInMilliSec() << endl;
    //     delete b;
    // }
    // classA* a1 = new classA(5);
    // delete a1;
    // classA* a2 = classA::createObject(6);
    // classA::destroyObject(a2);
    // classB* b1 = classB::createObject(5, 6);
    // classB::destroyObject(b1);
    return 0;
}