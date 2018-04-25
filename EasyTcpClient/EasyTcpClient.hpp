#ifndef _EasyTcpClient_hpp_
	#define _EasyTcpClient_hpp_
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
	#endif//_WIN32
	#include <stdio.h>
	#include "MessageHeader.hpp"


class EasyTcpClient
{
private:
	SOCKET _sock;

public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}

	//初始化socket
	void InitSocket()
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
			//printf("建立Socket<%d>成功...\n", _sock);
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//服务器定义的端口号
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);//inet_addr("127.0.0.1")--本机测试用;inet_addr("192.168.0.103");
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		//printf("<socket=%d>正在连接服务器<%s:%d>成功...\n", _sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误,连接服务器<%s:%d>失败...\n", _sock, ip , port);
		}
		else
		{
			//printf("<socket=%d>连接服务器<%s:%d>成功...\n", _sock, ip, port);
		}
		return ret;

	}

	//关闭套接字closesocket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			_sock = INVALID_SOCKET;
#ifdef _WIN32
			// 7 关闭套接字closesocket
			closesocket(_sock);
			//----------------以上是socket网络通信部分
			//清除Windows socket环境
			WSACleanup();//与WSAStartup(...)是一一对应关系
#else
			close(_sock);
#endif
		}
	}
	
	//处理查询网络消息
	int _nCount = 0;
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = /*{ 1,0 }*/{0,0};
			int ret = select(_sock + 1, &fdReads, 0, 0, /*NULL*/&t);
			//printf("select ret = %d   count = %d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("<socket=%d>select任务结束1\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束2\n", _sock);
					Close();
					return false;
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
	//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif	// !RECV_BUFF_SIZE
	//接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};//双缓冲
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE*10] = {};
	//消息缓冲区的数据尾部位置
	int _lastPos = 0;
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET cSock)
	{
		//5 接收数据
		int nLen = (int)recv(cSock, _szRecv, RECV_BUFF_SIZE, 0);
		//printf("nLen = %d\n", nLen);
		if (nLen <= 0)
		{
			printf("<socket=%d>与服务器断开连接，任务结束。\n", cSock);
			return -1;
		}
		//将收取的数据拷贝消息缓冲区
		memcpy(_szMsgBuf+ _lastPos, _szRecv, nLen);
		
	
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				LoginResult* login = (LoginResult*)header;
				//printf("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n", _sock, login->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				LogoutResult* logout = (LogoutResult*)header;
				//printf("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _sock, logout->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userjoin = (NewUserJoin*)header;
				//printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _sock, userjoin->dataLength);
			}
			break;
			case CMD_ERROR:
			{
				printf("<socket=%d>收到服务端消息：CMD_ERROR,数据长度：%d\n", _sock, header->dataLength);
			}
			break;
			default:
			{
				printf("<socket=%d>收到未定义消息,数据长度：%d\n", _sock, header->dataLength);
			}
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}



};


#endif // !_EasyTcpClient_hpp_
