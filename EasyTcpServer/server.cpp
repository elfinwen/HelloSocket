#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")


int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);//版本号，对应上面的头文件Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//与WSACleanup()是一一对应关系
	//----------------以下是socket网络通信部分
	//-- 用Socket API建立简易TCP服务端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2 bind 绑定用于接受客户端连接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");inet_addr("192.168.0.103");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误,绑定网络端口失败...\n");
	}
	else
	{
		printf("绑定网络端口成功...\n");
	}
	// 3 listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))//最多可以5个人同时进行连接
	{
		printf("错误,监听网络端口失败...\n");
	}
	else
	{
		printf("监听网络端口成功...\n");
	}
	// 4 accept 等待接收客户端连接
	sockaddr_in clinetAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	char msgBuf[] = "Hello, I'm Server.\n";
	_cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误,接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入：socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
	
	char _recvBuf[128] = {};
	while (true)
	{
		//5 接收客户端的数据
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("客户端已退出，任务结束。\n");
			break;
		}
		printf("收到命令：%s\n", _recvBuf);
		//6 处理请求
		if (0 == strcmp(_recvBuf, "getName"))
		{
			//7 向客户端返回请求数据
			memcpy(msgBuf, "xiao qiang.",sizeof("xiao qiang."));
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);

		}
		else if (0 == strcmp(_recvBuf, "getAge"))
		{
			memcpy(msgBuf, "80.", sizeof("80."));
			//7 向客户端返回请求数据
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else
		{
			memcpy(msgBuf, "???.", sizeof("???."));
			//7 向客户端返回请求数据
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);

		}
	}
	
	// 8 关闭套接字closesocket
	closesocket(_sock);
	//----------------以上是socket网络通信部分
	//清除Windows socket环境
	WSACleanup();//与WSAStartup(...)是一一对应关系
	printf("已退出，任务结束。\n");
	getchar();

	return 0;
}