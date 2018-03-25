#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")
enum CMD
{
	CMD_LOGIN,
	CMD_LOGINOUT,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;	//���ݳ���
	short cmd;			//����
};
//DataPackage
struct Login
{
	char userName[32];
	char passWord[32];
};
struct LoginResult
{
	int result;
};
struct Logout
{
	char userName[32];

};
struct LogoutResult
{
	int result;
};


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
	printf("�¿ͻ��˼��룺socket = %d,IP = %s\n", (int)_cSock, inet_ntoa(clinetAddr.sin_addr));
	
	
	while (true)
	{
		DataHeader header = {};
		//5 ���տͻ��˵�����
		int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("�ͻ������˳������������\n");
			break;
		}
		printf("�յ����%d  ���ݳ��ȣ�%d\n", header.cmd, header.dataLength);
		//6 ��������
		switch (header.cmd)
		{
			case CMD_LOGIN:
			{
				Login login = {};
				recv(_cSock, (char*)&login, sizeof(Login), 0);
				//�����ж��û��������Ƿ������Ĺ���
				LoginResult ret = {1};
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGINOUT:
			{
				Logout logout = {};
				recv(_cSock, (char*)&logout, sizeof(Logout), 0);
				//�����ж��û��������Ƿ������Ĺ���
				LogoutResult ret = {1};
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

			}
			break;

		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (char*)&header, sizeof(DataHeader), 0);
			break;
		}
	}
	
	// 8 �ر��׽���closesocket
	closesocket(_sock);
	//----------------������socket����ͨ�Ų���
	//���Windows socket����
	WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ
	printf("���˳������������\n");
	getchar();

	return 0;
}