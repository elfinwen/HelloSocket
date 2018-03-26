#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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
		printf("�ͻ������ӷ������ɹ�...");
	}
	
	
	while (true)
	{
		// 3 �û�������������
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4 ��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�յ�exit������������\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			// 5 ���������������
			Login login;
			strcpy(login.userName, "lyc");
			strcpy(login.passWord, "lycmm");
			send(_sock, (const char*)&login, sizeof(login), 0);
			// ���շ��������ص�����
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n",loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			// 5 ���������������
			Logout logout;
			strcpy(logout.userName, "lyc");
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			// 6 ���շ��������ص�����
			LoginResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutResult: %d\n", logoutRet.result);
		}
		else
		{
			printf("��֧�ֵ�������������롣\n");
		}
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