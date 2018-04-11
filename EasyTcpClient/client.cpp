#include "EasyTcpClient.hpp"
#include <thread>

void cmdThread(EasyTcpClient *client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "lyc");
			strcpy(login.passWord, "lydmm");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyc");
			client->SendData(&logout);
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
	//同时与多个服务端通信连接
	EasyTcpClient client1;
	client1.Connect("127.0.0.1", 4567);//192.168.74.1

	EasyTcpClient client2;
	client2.Connect("192.168.74.135", 4567);

	EasyTcpClient client3;
	client3.Connect("192.168.74.134", 4567);
/*
	//启动UI线程
	std::thread t1(cmdThread, &client1);
	t1.detach();//与主线程分离

	std::thread t2(cmdThread, &client2);
	t2.detach();//与主线程分离

	std::thread t3(cmdThread, &client3);
	t3.detach();//与主线程分离
*/
	Login login;
	strcpy(login.userName, "lyc");
	strcpy(login.passWord, "lydmm");
	
	while (client1.isRun() || client2.isRun() || client3.isRun())
	{
		client1.OnRun();
		client2.OnRun();
		client3.OnRun();

		client1.SendData(&login);
		client2.SendData(&login);
		client3.SendData(&login);
	}
	client1.Close();
	client2.Close();
	client3.Close();
	printf("已退出\n");
	getchar();
	return 0;
}