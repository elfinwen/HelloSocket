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
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clinetAddr, (socklen_t *)&nAddrLen);
#endif // _WIN32
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>����,���յ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("<socket=%d>�¿ͻ��˼��룺socket = %d,IP = %s\n", (int)_sock, (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
		}
		return (int)_cSock;
	}
	
	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8 �ر��׽���closesocket
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}
			closesocket(_sock);
			//----------------������socket����ͨ�Ų���
			//���Windows socket����
			WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ
#else
			// 8 �ر��׽���closesocket
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}

	}

	//����������Ϣ
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

			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
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
			//int ret = select(_sock + 1, &fdRead, 0, 0, &t);
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

			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))//����������
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

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[1024] = {};
		//5 ���տͻ��˵�����
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("�ͻ���<SOCKET=%d>���˳������������\n", _cSock);
			return -1;
		}

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;

	}
	
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	{
		//6 ��������
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("�յ��ͻ���<SOCKET=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
				//�����ж��û��������Ƿ������Ĺ���
				LoginResult ret;
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout* logout = (Logout*)header;
				printf("�յ��ͻ���<SOCKET=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s\n", _cSock, logout->dataLength, logout->userName);
				//�����ж��û��������Ƿ������Ĺ���
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
	
	//����ָ��socket����
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ��
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}

};




#endif // !_EasyTcpServer_hpp_