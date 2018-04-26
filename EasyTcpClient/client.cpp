#include "EasyTcpClient.hpp"
#include <thread>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}
}

//客户端数量
const int cCount = 1000/*FD_SETSIZE-1*/;
//发送线程数量
const int tCount = 4;
//客户端数组
EasyTcpClient* client[cCount];

void sendThread(int id)//4个线程id：1~4
{
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id * c;
	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("127.0.0.1", 4567);//192.168.74.1
		printf("Connect=%d\n", n);
	}

	Login login;
	strcpy(login.userName, "lyc");
	strcpy(login.passWord, "lydmm");
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->SendData(&login);
			//client[n]->OnRun();
		}
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}
}

int main()
{
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();//与主线程分离

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();//与主线程分离
	}

	while (g_bRun)
		Sleep(100);
	
	
	printf("已退出\n");
	//getchar();
	return 0;
}