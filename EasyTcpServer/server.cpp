#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib,"ws2_32.lib")
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
struct Login: public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
struct LoginResult: public DataHeader
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


std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
	//缓冲区
	char szRecv[1024] = {};
	//5 接收客户端的数据
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("客户端<SOCKET=%d>已退出，任务结束。\n", _cSock);
		return -1;
	}
	//6 处理请求
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("收到客户端<SOCKET=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
		//忽略判断用户名密码是否正常的过程
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("收到客户端<SOCKET=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s\n", _cSock, logout->dataLength, logout->userName);
		//忽略判断用户名密码是否正常的过程
		LogoutResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

	}
	break;

	default:
	{
		DataHeader _header = { 0, CMD_ERROR };
		send(_cSock, (char*)&_header, sizeof(DataHeader), 0);
	}
	break;
	}

	return 0;

}

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
	
	while (true)
	{
		//伯克利 BSD socket 
		//fd_set:描述符（socket）集合。最多同时处理64个socket  因为 #define FD_SETSIZE  64 （F12查看fd_set）
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		//FD_ZERO 清理描述符（socket）集合：指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，如果不清空，
		//由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		//FD_SET：将描述符_sock加入描述符集合fdRead、fdWrite、fdExp中
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size()-1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		
		///select函数第一个参数nfds 是一个整数值  是指fd_set集合中所有描述符（socket）的范围，而不是数量。
		///即是所有文件描述符最大值+1  在Windows中这个参数可以写0（因为windows中已经处理了）即windows中可
		///以写成select(0, &fdRead, &fdWrite, &fdExp, NULL)。
		///该值主要对非windows有意义 如linux等
		///select函数最后一个参数为timeout为NULL,为阻塞模式;为0,则是非阻塞的（非阻塞：仅检测描述符集合的状态，然后立即返回，并不
		///等待外部事件的发生）；timeout所指向的结构设为非零时间（等待固定时间：如果在指定的时间段里有事件发生或者时间耗尽，函数均返回）
		timeval t = /*{1,0}*/ { 0,0 };//第一个值1，表示最大的时间值为1秒，并不是说他一定要等到1秒
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t/*NULL*/);
		if (ret < 0)
		{
			printf("select任务结束。\n");
			break;
		}
		//FD_ISSET：判断描述符_sock是否在描述符集合fdRead中
		if (FD_ISSET(_sock, &fdRead))//集合中有我
		{
			FD_CLR(_sock, &fdRead); ///FD_CLR 用于在文件描述符集合中删除一个文件描述符
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
			else
			{
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_cSock);
				printf("新客户端加入：socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
			}
			
			
		}
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}

		printf("空闲时间处理其他业务..\n");
	}

	// 8 关闭套接字closesocket
	for (size_t n = g_clients.size()-1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	
	//closesocket(_sock);
	//----------------以上是socket网络通信部分
	//清除Windows socket环境
	WSACleanup();//与WSAStartup(...)是一一对应关系
	printf("已退出，任务结束。\n");
	getchar();

	return 0;
}