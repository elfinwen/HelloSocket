#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include <unistd.h> //uni std
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <thread>



enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;	//数据长度
	short cmd;			//命令
};
//DataPackage
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};
struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];

};
struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}

	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}

	int scok;//新用户的socket
};
int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};
	//5 接收客户端的数据
	int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("与服务器断开连接，任务结束。\n");
		return -1;
	}
	//6 处理请求
	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginResult* loginresult = (LoginResult*)szRecv;
			printf("收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LogoutResult* logoutresult = (LogoutResult*)szRecv;
			printf("收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* newuserjoin = (NewUserJoin*)szRecv;
			printf("收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _cSock, header->dataLength);

		}
		break;
		//default:
		//{
		//	DataHeader _header = { 0, CMD_ERROR };
		//	send(_cSock, (char*)&_header, sizeof(DataHeader), 0);
		//}
		//break;
	}

	return 0;

}
bool g_bRun = true;
void cmdThread(SOCKET sock)
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "lyc");
			strcpy(login.passWord, "lydmm");
			send(sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyc");
			send(sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
#ifdef _WIN32
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);//版本号，对应上面的头文件Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//与WSACleanup()是一一对应关系
#endif
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
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//inet_addr("127.0.0.1")--本机测试用;inet_addr("192.168.0.103");
#else
	_sin.sin_addr.s_addr = inet_addr("192.168.16.1");
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误,客户端连接服务器失败...\n");
	}
	else
	{
		printf("客户端连接服务器成功...\n");
	}
	
	//启动线程
	std::thread t1(cmdThread, _sock);
	t1.detach();//与主线程分离
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = {1,0}/*{0,0}*/;
		int ret = select(_sock+1, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select任务结束1\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);

			if (-1 == processor(_sock))
			{
				printf("select任务结束2\n");
				break;
			}
		}
	}
#ifdef _WIN32
	// 7 关闭套接字closesocket
	closesocket(_sock);
	//----------------以上是socket网络通信部分
	//清除Windows socket环境
	WSACleanup();//与WSAStartup(...)是一一对应关系
#else
	close(_sock);
#endif
	printf("已退出\n");
	getchar();

	return 0;
}