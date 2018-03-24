#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")

struct DataPackage
{
	int age;
	char name[32];
};
int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);//版本号，对应上面的头文件Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//与WSACleanup()是一一对应关系
	//----------------以下是socket网络通信部分
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，客户端建立Socket失败...\n");
	}
	else
	{
		printf("客户端建立Socket成功...\n");
	}
	// 2 连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//服务器定义的端口号
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//inet_addr("127.0.0.1")--本机测试用;inet_addr("192.168.0.103");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误,客户端连接服务器失败...\n");
	}
	else
	{
		printf("客户端连接服务器成功...");
	}
	
	
	while (true)
	{
		// 3 用户输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4 处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("收到exit命令，任务结束。\n");
			break;
		}
		else
		{
			// 5 向服务器发送请求
			send(_sock, cmdBuf, strlen(cmdBuf)+1, 0);
		}
		// 6 接收服务器信息 recv
		char recvBuf[256] = {};
		int nlen = recv(_sock, recvBuf, 256, 0);
		if (nlen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			printf("客户端接收到数据：年龄=%d ，姓名=%s\n", dp->age, dp->name);
		}
	}
	
	// 7 关闭套接字closesocket
	closesocket(_sock);

	//----------------以上是socket网络通信部分
	//清除Windows socket环境
	WSACleanup();//与WSAStartup(...)是一一对应关系
	printf("已退出\n");
	getchar();

	return 0;
}