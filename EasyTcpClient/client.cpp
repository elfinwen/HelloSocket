#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")

struct DataPackage
{
	int age;
	char name[32];
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
		else
		{
			// 5 ���������������
			send(_sock, cmdBuf, strlen(cmdBuf)+1, 0);
		}
		// 6 ���շ�������Ϣ recv
		char recvBuf[256] = {};
		int nlen = recv(_sock, recvBuf, 256, 0);
		if (nlen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			printf("�ͻ��˽��յ����ݣ�����=%d ������=%s\n", dp->age, dp->name);
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