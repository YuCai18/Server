// HelloThread.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<thread>
#include<mutex>

using namespace std;

mutex m;
const int tCount = 4;
//atomic_int sum = 0;
atomic<int> sum = 0;

void workFun(int index) {
    
    for (int n = 0;n < 10000;++n) {
        //m.lock();
        //cout << index << "Hello,other thread." << n << endl;
        //m.unlock();
        //lock_guard<mutex> lg(m); //自解锁
        sum++;
    }    
}
int main()
{
    thread t[tCount];
    for (int n = 0;n < tCount;++n)
        t[n] = thread(workFun,n);


    for (int n = 0;n < tCount;++n)
        t[n].join();
    cout << "sum=" << sum << endl;
    cout << "Hello,main thread." << endl;
    
    return 0;
}

