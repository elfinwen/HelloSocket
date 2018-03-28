#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>

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

	int scok;//���û���socket
};
int processor(SOCKET _cSock)
{
	//������
	char szRecv[1024] = {};
	//5 ���տͻ��˵�����
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ����������\n", _cSock);
		return -1;
	}
	//6 ��������
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginresult = (LoginResult*)szRecv;
<<<<<<< HEAD
		printf("�յ��������Ϣ��CMD_LOGIN_RESULT,���ݳ��ȣ�%d\n", header->dataLength);
=======
		printf("�յ��������Ϣ��CMD_LOGIN_RESULT,���ݳ��ȣ�%d\n", _cSock, header->dataLength);
>>>>>>> 84ed64739fc6db88a55ffb51676f22e01f2b876e
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutresult = (LogoutResult*)szRecv;
<<<<<<< HEAD
		printf("�յ��������Ϣ��CMD_LOGOUT_RESULT,���ݳ��ȣ�%d\n", header->dataLength);
=======
		printf("�յ��������Ϣ��CMD_LOGOUT_RESULT,���ݳ��ȣ�%d\n", _cSock, header->dataLength);
>>>>>>> 84ed64739fc6db88a55ffb51676f22e01f2b876e
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* newuserjoin = (NewUserJoin*)szRecv;
		printf("�յ��������Ϣ��CMD_NEW_USER_JOIN,���ݳ��ȣ�%d\n", _cSock, header->dataLength);

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
<<<<<<< HEAD
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
			printf("�˳�cmdThread�߳�\n");
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
			printf("��֧�ֵ����\n");
		}
	}
}

=======
>>>>>>> 84ed64739fc6db88a55ffb51676f22e01f2b876e
int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);//�汾�ţ���Ӧ�����ͷ�ļ�Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//��WSACleanup()��һһ��Ӧ��ϵ
	//----------------������socket����ͨ�Ų���
	//-- ��Socket API��������TCP�ͻ���
	// 1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("���󣬿ͻ��˽���Socketʧ��...\n");
	}
	else
	{
		printf("�ͻ��˽���Socket�ɹ�...\n");
	}
	// 2 ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//����������Ķ˿ں�
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//inet_addr("127.0.0.1")--����������;inet_addr("192.168.0.103");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("����,�ͻ������ӷ�����ʧ��...\n");
	}
	else
	{
		printf("�ͻ������ӷ������ɹ�...\n");
	}
	
	//�����߳�
	std::thread t1(cmdThread, _sock);
	t1.detach();//�����̷߳���
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
<<<<<<< HEAD
		timeval t = {1,0}/*{0,0}*/;
=======
		timeval t = /*{ 1, 0 }*/{0,0};
>>>>>>> 84ed64739fc6db88a55ffb51676f22e01f2b876e
		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select�������1\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);

			if (-1 == processor(_sock))
			{
				printf("select�������2\n");
				break;
			}
		}
<<<<<<< HEAD

		//printf("����ʱ�䴦������ҵ��..\n");
		
=======
		
		printf("����ʱ�䴦������ҵ��..\n");
		Login login;
		strcpy(login.userName, "lyc");
		strcpy(login.passWord, "lycmm");
		send(_sock,(const char*)&login, sizeof(Login), 0);
		//Sleep(1000);
>>>>>>> 84ed64739fc6db88a55ffb51676f22e01f2b876e

	}
	
	// 7 �ر��׽���closesocket
	closesocket(_sock);

	//----------------������socket����ͨ�Ų���
	//���Windows socket����
	WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ
	printf("���˳�\n");
	getchar();

	return 0;
}