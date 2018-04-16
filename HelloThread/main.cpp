#include <iostream>
#include <thread>
#include <mutex>//锁

using namespace std;

mutex m;
const int tCount = 4;
int sum = 0;
void workFun(int index)
{
	for (int n = 0; n < 200000; n++)
	{
		m.lock();//临界区域：被锁定的区域
		//临界区域-开始
		sum++;
		//临界区域-结束
		m.unlock();
	}
	//cout << index << "hello,other thread." << n << endl;	
}

int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
	}
	for (int n = 0; n < tCount; n++)
	{
		//t[n].detach();//detach:同时并行执行t[n]线程和主线程的任务，且互不影响（t[n]线程从main线程完全分离,抢占式分配资源）
		t[n].join();//join：线程t[n]完成之后，退出t[n]线程，才会再回到主线程，执行主线程的任务
	}
	cout << "sum=" << sum << endl;
	cout << "hello,main thread." << endl;

	system("pause");
	return 0;

}