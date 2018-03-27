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
	short dataLength;	//���ݳ���
	short cmd;			//����
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

	int scok;//���û���socket
};


std::vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
	//������
	char szRecv[1024] = {};
	//5 ���տͻ��˵�����
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("�ͻ���<SOCKET=%d>���˳������������\n", _cSock);
		return -1;
	}
	//6 ��������
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ��ͻ���<SOCKET=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s,passWord = %s\n", _cSock, login->dataLength, login->userName, login->passWord);
		//�����ж��û��������Ƿ������Ĺ���
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
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

	return 0;

}

int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);//�汾�ţ���Ӧ�����ͷ�ļ�Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//��WSACleanup()��һһ��Ӧ��ϵ
	//----------------������socket����ͨ�Ų���
	//-- ��Socket API��������TCP�����
	// 1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");inet_addr("192.168.0.103");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("����,������˿�ʧ��...\n");
	}
	else
	{
		printf("������˿ڳɹ�...\n");
	}
	// 3 listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5))//������5����ͬʱ��������
	{
		printf("����,��������˿�ʧ��...\n");
	}
	else
	{
		printf("��������˿ڳɹ�...\n");
	}
	
	while (true)
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

		for (int n = (int)g_clients.size()-1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		
		///select������һ������nfds ��һ������ֵ  ��ָfd_set������������������socket���ķ�Χ��������������
		///���������ļ����������ֵ+1  ��Windows�������������д0����Ϊwindows���Ѿ������ˣ���windows�п�
		///��д��select(0, &fdRead, &fdWrite, &fdExp, NULL)��
		///��ֵ��Ҫ�Է�windows������ ��linux��
		///select�������һ������ΪtimeoutΪNULL,Ϊ����ģʽ;Ϊ0,���Ƿ������ģ�����������������������ϵ�״̬��Ȼ���������أ�����
		///�ȴ��ⲿ�¼��ķ�������timeout��ָ��Ľṹ��Ϊ����ʱ�䣨�ȴ��̶�ʱ�䣺�����ָ����ʱ��������¼���������ʱ��ľ������������أ�
		timeval t = /*{1,0}*/ { 0,0 };//��һ��ֵ1����ʾ����ʱ��ֵΪ1�룬������˵��һ��Ҫ�ȵ�1��
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t/*NULL*/);
		if (ret < 0)
		{
			printf("select���������\n");
			break;
		}
		//FD_ISSET���ж�������_sock�Ƿ�������������fdRead��
		if (FD_ISSET(_sock, &fdRead))//����������
		{
			FD_CLR(_sock, &fdRead); ///FD_CLR �������ļ�������������ɾ��һ���ļ�������
			// 4 accept �ȴ����տͻ�������
			sockaddr_in clinetAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			char msgBuf[] = "Hello, I'm Server.\n";
			_cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{
				printf("����,���յ���Ч�ͻ���SOCKET...\n");
			}
			else
			{
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_cSock);
				printf("�¿ͻ��˼��룺socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
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

		printf("����ʱ�䴦������ҵ��..\n");
	}

	// 8 �ر��׽���closesocket
	for (size_t n = g_clients.size()-1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	
	//closesocket(_sock);
	//----------------������socket����ͨ�Ų���
	//���Windows socket����
	WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ
	printf("���˳������������\n");
	getchar();

	return 0;
}