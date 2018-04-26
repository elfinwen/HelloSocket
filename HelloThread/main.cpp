#include <iostream>
#include <thread>
#include <mutex>//��
#include <atomic>//ԭ��
#include "CELLTimestamp.hpp"

using namespace std;
//ԭ�Ӳ���  ԭ�� ����

mutex m;
const int tCount = 4;
atomic_int sum = 0;//ԭ��int��   atomic<int> sum = 0;

void workFun(int index)
{
	for (int n = 0; n < 20000000; n++)
	{
		//�Խ���
		//lock_guard<mutex> lg(m);
		//m.lock();//�ٽ����򣺱�����������
		//�ٽ�����-��ʼ
		sum++;
		//�ٽ�����-����
		//m.unlock();
	}//�̰߳�ȫ  �̲߳���ȫ
	//ԭ�Ӳ������������������ʱ��С�Ĳ�����λ
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
		//t[n].detach();//detach:ͬʱ����ִ��t[n]�̺߳����̵߳������һ���Ӱ�죨t[n]�̴߳�main�߳���ȫ����,��ռʽ������Դ��
		t[n].join();//join���߳�t[n]���֮���˳�t[n]�̣߳��Ż��ٻص����̣߳�ִ�����̵߳�����
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",���߳�sum=" << sum << endl;
	sum = 0;
	tTime.update();
	for (int n = 0; n < 80000000; n++)
	{
		sum++;
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",���߳�sum=" << sum << endl;
	cout << "hello,main thread." << endl;

	system("pause");
	return 0;

}