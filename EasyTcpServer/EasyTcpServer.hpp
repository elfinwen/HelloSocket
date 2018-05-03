#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      2506 //本句要放在#include <WinSock2.h>之前。因为<WinSock2.h>中有表示，如果没有定义FD_SETSIZE，则 #define FD_SETSIZE 64
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
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240 //10k
#endif	// !RECV_BUFF_SIZE

#define _CellServer_THREAD_COUNT 4

class ClientSocket 
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}
	char *msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	//fd_set file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//消息缓冲区的数据尾部位置
	int _lastPos;
};

class INetEvent
{
public:
	//客户端离开事件(纯虚函数)
	virtual void OnLeave(ClientSocket* pClient) = 0;
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) = 0;

private:

};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;

	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//处理网络消息
	bool OnRun()
	{
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)//C++11新方法：快速遍历
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);//1毫秒
				std::this_thread::sleep_for(t);//本线程休眠1毫秒
				continue;
			}
				
			//伯克利 BSD socket 
			//fd_set:描述符（socket）集合。最多同时处理64个socket  因为 #define FD_SETSIZE  64 （F12查看fd_set）
			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExp;
			//FD_ZERO 清理描述符（socket）集合：指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，如果不清空，
			//由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//FD_SET：将描述符_sock加入描述符集合fdRead、fdWrite、fdExp中
			//FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);
			//将描述符（socket）加入集合
			SOCKET maxSock = _clients[0]->sockfd();

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			///select函数第一个参数nfds 是一个整数值  是指fd_set集合中所有描述符（socket）的范围，而不是数量。
			///即是所有文件描述符最大值+1  在Windows中这个参数可以写0（因为windows中已经处理了）即windows中可
			///以写成select(0, &fdRead, &fdWrite, &fdExp, NULL)。
			///该值主要对非windows有意义 如linux等
			///select函数最后一个参数为timeout为NULL,为阻塞模式;为0,则是非阻塞的（非阻塞：仅检测描述符集合的状态，然后立即返回，并不
			///等待外部事件的发生）；timeout所指向的结构设为非零时间（等待固定时间：如果在指定的时间段里有事件发生或者时间耗尽，函数均返回）
			//timeval t = { 1,0 } /*{ 0,0 }*/;//第一个值1，表示最大的时间值为1秒，并不是说他一定要等到1秒
			int ret = select(_sock + 1, &fdRead, nullptr/*&fdWrite*/, nullptr/*&fdExp*/, nullptr/*&t*//*NULL*/);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}
			////FD_ISSET：判断描述符_sock是否在描述符集合fdRead中
			//if (FD_ISSET(_sock, &fdRead))//集合中有我
			//{
			//	FD_CLR(_sock, &fdRead); ///FD_CLR 用于在文件描述符集合中删除一个文件描述符
			//	Accept();
			//	return true;

			//}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))//集合中有我
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != _clients.end())
						{
							if(_pNetEvent)
								_pNetEvent->OnLeave(_clients[n]);
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8 关闭套接字closesocket
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			//----------------以上是socket网络通信部分
			//清除Windows socket环境
			WSACleanup();//与WSAStartup(...)是一一对应关系
#else
			// 8 关闭套接字closesocket
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
#endif
			_clients.clear();
		}

	}

	//缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
		//5 接收客户端的数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("客户端<SOCKET=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);

			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}


		return 0;

	}

	//响应网络消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		_pNetEvent->OnNetMsg(cSock, header);
		
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("收到客户端<SOCKET=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
			//忽略判断用户名密码是否正常的过程
			/*LoginResult ret;
			SendData(cSock, &ret);*/
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<SOCKET=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s\n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正常的过程
			/*LogoutResult ret;
			SendData(cSock, &ret);*/
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息,数据长度：%d\n", cSock, header->dataLength);
			/*	DataHeader ret;
			SendData(cSock, &ret);*/
		}
		break;
		}
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);//自解锁
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	SOCKET _sock;
	//正式客户队列
	std::vector<ClientSocket*> _clients;
	//缓冲客户队列
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	INetEvent* _pNetEvent;

public:
	std::atomic_int _recvCount;
	

};



//new 堆内存（堆内存：根据电脑的内存条大小决定，如2G ，4G，甚至更大）；没有通过new创建的（如  int a = 0 ），在栈空间上（C++的栈空间很小，一般只有1M~2M,根据系统
//不同，可能会有些许区别）
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime;
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
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clinetAddr, (socklen_t *)&nAddrLen);
#endif // _WIN32
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>错误,接收到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			addClientToCellServer(new ClientSocket(cSock));
			//printf("<socket=%d>新客户端<%d>加入：socket = %d,IP = %s\n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clinetAddr.sin_addr));
		}
		return (int)cSock;
	}

	void addClientToCellServer(ClientSocket* pClient)
	{
		_clients.push_back(pClient);
		//查找客户数量最少的CellServer消息处理对象
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
	}

	void Start()
	{
		for (int n = 0; n < _CellServer_THREAD_COUNT; n++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	}

	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8 关闭套接字closesocket
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			//----------------以上是socket网络通信部分
			//清除Windows socket环境
			WSACleanup();//与WSAStartup(...)是一一对应关系
#else
			// 8 关闭套接字closesocket
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
#endif
			_clients.clear();
		}

	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//伯克利 BSD socket 
			//fd_set:描述符（socket）集合。最多同时处理64个socket  因为 #define FD_SETSIZE  64 （F12查看fd_set）
			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExp;
			//FD_ZERO 清理描述符（socket）集合：指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，如果不清空，
			//由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//FD_SET：将描述符_sock加入描述符集合fdRead、fdWrite、fdExp中
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);
			

			///select函数第一个参数nfds 是一个整数值  是指fd_set集合中所有描述符（socket）的范围，而不是数量。
			///即是所有文件描述符最大值+1  在Windows中这个参数可以写0（因为windows中已经处理了）即windows中可
			///以写成select(0, &fdRead, &fdWrite, &fdExp, NULL)。
			///该值主要对非windows有意义 如linux等
			///select函数最后一个参数为timeout为NULL,为阻塞模式;为0,则是非阻塞的（非阻塞：仅检测描述符集合的状态，然后立即返回，并不
			///等待外部事件的发生）；timeout所指向的结构设为非零时间（等待固定时间：如果在指定的时间段里有事件发生或者时间耗尽，函数均返回）
			timeval t = { 1,0 } /*{ 0,0 }*/;//第一个值1，表示最大的时间值为1秒，并不是说他一定要等到1秒
			int ret = select(_sock + 1, &fdRead, 0/*&fdWrite*/, 0/*&fdExp*/, &t/*NULL*/);
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
				return true;
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
	
	//缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
		//5 接收客户端的数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("客户端<SOCKET=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				//OnNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}

		
		return 0;

	}
	
	//响应网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			for (auto ser : _cellServers)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
				
			printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,_recvCount<%d>\n",_cellServers.size(), t1, _sock, _clients.size(),recvCount);
			_tTime.update();
		}
	}
	
	//发送指定socket数据
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//群发
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

	virtual void OnLeave(ClientSocket* pClient)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (_clients[n] == pClient)
			{
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
					_clients.erase(iter);
			}
		}
	}

	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		
	}
};




#endif // !_EasyTcpServer_hpp_