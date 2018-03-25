#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;	//数据长度
	short cmd;			//命令
};
//DataPackage
struct Login
{
	char userName[32];
	char passWord[32];
};
struct LoginResult
{
	int result;
};
struct Logout
{
	char userName[32];

};
struct LogoutResult
{
	int result;
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login = {"lyc", "lycmm"};
			DataHeader dh = {sizeof(login), CMD_LOGIN};
			// 5 向服务器发送请求
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&login, sizeof(login), 0);
			// 接收服务器返回的数据
			DataHeader retHeader = {};
			LoginResult loginRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n",loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout = {"lyc"};
			DataHeader dh = {sizeof(logout), CMD_LOGOUT};
			// 5 向服务器发送请求
			send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			// 6 接收服务器返回的数据
			DataHeader retHeader = {};
			LoginResult logoutRet = {};
			recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutResult: %d\n", logoutRet.result);
		}
		else
		{
			printf("不支持的命令，请重新输入。\n");
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