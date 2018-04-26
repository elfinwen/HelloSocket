#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      4024 //����Ҫ����#include <WinSock2.h>֮ǰ����Ϊ<WinSock2.h>���б�ʾ�����û�ж���FD_SETSIZE���� #define FD_SETSIZE 64
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
#include "CELLTimestamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240 //10k
#endif	// !RECV_BUFF_SIZE

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
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//��Ϣ������������β��λ��
	int _lastPos;
};

//new ���ڴ棨���ڴ棺���ݵ��Ե��ڴ�����С��������2G ��4G���������󣩣�û��ͨ��new�����ģ���  int a = 0 ������ջ�ռ��ϣ�C++��ջ�ռ��С��һ��ֻ��1M~2M,����ϵͳ
//��ͬ�����ܻ���Щ������
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	CELLTimestamp _tTime;
	int _recvCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);//�汾�ţ���Ӧ�����ͷ�ļ�Winsock2.h  
		WSADATA dat;
		WSAStartup(ver, &dat);//��WSACleanup()��һһ��Ӧ��ϵ
#endif
							  // 1 ����һ��socket
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Socketʧ��...\n");
		}
		else
		{
			printf("����Socket<%d>�ɹ�...\n", _sock);
		}
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
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
			printf("����,������˿�<%d>ʧ��...\n",port);
		}
		else
		{
			printf("������˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}
	
	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)//������5����ͬʱ��������
		{
			printf("<socket=%d>����,��������˿�ʧ��...\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>��������˿ڳɹ�...\n", (int)_sock);
		}
		return ret;
	}
	
	//���ܿͻ�������
	int Accept()
	{
		// 4 accept �ȴ����տͻ�������
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
			printf("<socket=%d>����,���յ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			//printf("<socket=%d>�¿ͻ���<%d>���룺socket = %d,IP = %s\n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clinetAddr.sin_addr));
		}
		return (int)cSock;
	}
	
	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8 �ر��׽���closesocket
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			//----------------������socket����ͨ�Ų���
			//���Windows socket����
			WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ
#else
			// 8 �ر��׽���closesocket
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

	//����������Ϣ
	int _nCount = 0;
	bool OnRun()
	{
		if (isRun())
		{
			//������ BSD socket 
			//fd_set:��������socket�����ϡ����ͬʱ����64��socket  ��Ϊ #define FD_SETSIZE  64 ��F12�鿴fd_set��
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			//FD_ZERO ������������socket�����ϣ�ָ�����ļ�����������գ��ڶ��ļ����������Ͻ�������ǰ�����������г�ʼ�����������գ�
			//������ϵͳ�����ڴ�ռ��ͨ����������մ������Խ���ǲ���֪��
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//FD_SET����������_sock��������������fdRead��fdWrite��fdExp��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			///select������һ������nfds ��һ������ֵ  ��ָfd_set������������������socket���ķ�Χ��������������
			///���������ļ����������ֵ+1  ��Windows�������������д0����Ϊwindows���Ѿ������ˣ���windows�п�
			///��д��select(0, &fdRead, &fdWrite, &fdExp, NULL)��
			///��ֵ��Ҫ�Է�windows������ ��linux��
			///select�������һ������ΪtimeoutΪNULL,Ϊ����ģʽ;Ϊ0,���Ƿ������ģ�����������������������ϵ�״̬��Ȼ���������أ�����
			///�ȴ��ⲿ�¼��ķ�������timeout��ָ��Ľṹ��Ϊ����ʱ�䣨�ȴ��̶�ʱ�䣺�����ָ����ʱ��������¼���������ʱ��ľ������������أ�
			timeval t = { 1,0 } /*{ 0,0 }*/;//��һ��ֵ1����ʾ����ʱ��ֵΪ1�룬������˵��һ��Ҫ�ȵ�1��
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t/*NULL*/);
			//printf("select ret = %d   count = %d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
			//FD_ISSET���ж�������_sock�Ƿ�������������fdRead��
			if (FD_ISSET(_sock, &fdRead))//����������
			{
				FD_CLR(_sock, &fdRead); ///FD_CLR �������ļ�������������ɾ��һ���ļ�������
				Accept();
				return true;

			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))//����������
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	
	//������
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{
		//5 ���տͻ��˵�����
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("�ͻ���<SOCKET=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�����ݿ�����Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient->sockfd(), header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}

		
		return 0;

	}
	
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>,clients<%d>,_recvCount<%d>\n", t1, _sock, _clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		//6 ��������
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				//printf("�յ��ͻ���<SOCKET=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
				//�����ж��û��������Ƿ������Ĺ���
				/*LoginResult ret;
				SendData(cSock, &ret);*/
			}
			break;
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
				//printf("�յ��ͻ���<SOCKET=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s\n", cSock, logout->dataLength, logout->userName);
				//�����ж��û��������Ƿ������Ĺ���
				/*LogoutResult ret;
				SendData(cSock, &ret);*/
			}
			break;
			default:
			{
				printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", cSock, header->dataLength);
			/*	DataHeader ret;
				SendData(cSock, &ret);*/
			}
			break;
		}
	}
	
	//����ָ��socket����
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ��
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

};




#endif // !_EasyTcpServer_hpp_