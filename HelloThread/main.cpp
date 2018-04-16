#include <iostream>
#include <thread>
#include <mutex>//��

using namespace std;

mutex m;
const int tCount = 4;
int sum = 0;
void workFun(int index)
{
	for (int n = 0; n < 200000; n++)
	{
		m.lock();//�ٽ����򣺱�����������
		//�ٽ�����-��ʼ
		sum++;
		//�ٽ�����-����
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
		//t[n].detach();//detach:ͬʱ����ִ��t[n]�̺߳����̵߳������һ���Ӱ�죨t[n]�̴߳�main�߳���ȫ����,��ռʽ������Դ��
		t[n].join();//join���߳�t[n]���֮���˳�t[n]�̣߳��Ż��ٻص����̣߳�ִ�����̵߳�����
	}
	cout << "sum=" << sum << endl;
	cout << "hello,main thread." << endl;

	system("pause");
	return 0;

}