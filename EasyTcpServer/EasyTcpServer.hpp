#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
#endif	// !_WIN32

#include <stdio.h>
//#include <thread>
#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);//版本号，对应上面的头文件Winsock2.h  
		WSADATA dat;
		WSAStartup(ver, &dat);//与WSACleanup()是一一对应关系
#endif
							  // 1 建立一个socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败...\n");
		}
		else
		{
			printf("建立Socket<%d>成功...\n", _sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind 绑定用于接受客户端连接的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");inet_addr("192.168.0.103");
		}
		
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");inet_addr("192.168.0.103");
		}
#endif // _WIN32
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("错误,绑定网络端口<%d>失败...\n",port);
		}
		else
		{
			printf("绑定网络端口<%d>成功...\n",port);
		}
		return ret;
	}
	
	//监听端口号
	int Listen(int n)
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)//最多可以5个人同时进行连接
		{
			printf("<socket=%d>错误,监听网络端口失败...\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>监听网络端口成功...\n", (int)_sock);
		}
		return ret;
	}
	
	//接受客户端连接
	int Accept()
	{
		// 4 accept 等待接收客户端连接
		sockaddr_in clinetAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clinetAddr, (socklen_t *)&nAddrLen);
#endif // _WIN32
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>错误,接收到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("<socket=%d>新客户端加入：socket = %d,IP = %s\n", (int)_sock, (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
		}
		return (int)_cSock;
	}
	
	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8 关闭套接字closesocket
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}
			closesocket(_sock);
			//----------------以上是socket网络通信部分
			//清除Windows socket环境
			WSACleanup();//与WSAStartup(...)是一一对应关系
#else
			// 8 关闭套接字closesocket
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}

	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
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
			SOCKET maxSock = _sock;

			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}

			///select函数第一个参数nfds 是一个整数值  是指fd_set集合中所有描述符（socket）的范围，而不是数量。
			///即是所有文件描述符最大值+1  在Windows中这个参数可以写0（因为windows中已经处理了）即windows中可
			///以写成select(0, &fdRead, &fdWrite, &fdExp, NULL)。
			///该值主要对非windows有意义 如linux等
			///select函数最后一个参数为timeout为NULL,为阻塞模式;为0,则是非阻塞的（非阻塞：仅检测描述符集合的状态，然后立即返回，并不
			///等待外部事件的发生）；timeout所指向的结构设为非零时间（等待固定时间：如果在指定的时间段里有事件发生或者时间耗尽，函数均返回）
			timeval t = { 1,0 } /*{ 0,0 }*/;//第一个值1，表示最大的时间值为1秒，并不是说他一定要等到1秒
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t/*NULL*/);
			//int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}
			//FD_ISSET：判断描述符_sock是否在描述符集合fdRead中
			if (FD_ISSET(_sock, &fdRead))//集合中有我
			{
				FD_CLR(_sock, &fdRead); ///FD_CLR 用于在文件描述符集合中删除一个文件描述符
				Accept();

			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))//集合中有我
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	
	//接受数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[1024] = {};
		//5 接收客户端的数据
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("客户端<SOCKET=%d>已退出，任务结束。\n", _cSock);
			return -1;
		}

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;

	}
	
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		//6 处理请求
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("收到客户端<SOCKET=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
				//忽略判断用户名密码是否正常的过程
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
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
	}
	
	//发送指定socket数据
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//群发
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}

};




#endif // !_EasyTcpServer_hpp_