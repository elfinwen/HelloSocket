#include <iostream>
#include <thread>
#include <mutex>//锁
#include <atomic>//原子
#include "CELLTimestamp.hpp"

using namespace std;
//原子操作  原子 分子

mutex m;
const int tCount = 4;
atomic_int sum = 0;//原子int型   atomic<int> sum = 0;

void workFun(int index)
{
	for (int n = 0; n < 20000000; n++)
	{
		//自解锁
		//lock_guard<mutex> lg(m);
		//m.lock();//临界区域：被锁定的区域
		//临界区域-开始
		sum++;
		//临界区域-结束
		//m.unlock();
	}//线程安全  线程不安全
	//原子操作：计算机处理命令时最小的操作单位
	//cout << index << "hello,other thread." << n << endl;	
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
		//t[n].detach();//detach:同时并行执行t[n]线程和主线程的任务，且互不影响（t[n]线程从main线程完全分离,抢占式分配资源）
		t[n].join();//join：线程t[n]完成之后，退出t[n]线程，才会再回到主线程，执行主线程的任务
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",多线程sum=" << sum << endl;
	sum = 0;
	tTime.update();
	for (int n = 0; n < 80000000; n++)
	{
		sum++;
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",主线程sum=" << sum << endl;
	cout << "hello,main thread." << endl;

	system("pause");
	return 0;

}