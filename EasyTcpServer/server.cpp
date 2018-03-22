#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")


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
	_cSock = accept(_sock, (sockaddr*)&clinetAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("����,���յ���Ч�ͻ���SOCKET...\n");
	}
	// 5 send ��ͻ��˷���һ������
	char msgBuf[] = "Hello, I'm Server.\n";
	send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
	// 6 �ر��׽���closesocket
	closesocket(_sock);


	//----------------������socket����ͨ�Ų���
	//���Windows socket����
	WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ

	return 0;
}