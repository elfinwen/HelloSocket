#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")


int main()
{
	//����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);//�汾�ţ���Ӧ�����ͷ�ļ�Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//��WSACleanup()��һһ��Ӧ��ϵ
	//----------------������socket����ͨ�Ų���
	//-- ��Socket API��������TCP�ͻ���
	// 1 ����һ��socket
	// 2 ���ӷ����� connect
	// 3 ���շ�������Ϣ recv
	// 4 �ر��׽���closesocket
	//-- ��Socket API��������TCP�����
	// 1 ����һ��socket
	// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
	// 3 listen ��������˿�
	// 4 accept �ȴ����տͻ�������
	// 5 send ��ͻ��˷���һ������
	// 6 �ر��׽���closesocket


	//----------------������socket����ͨ�Ų���
	//���Windows socket����
	WSACleanup();//��WSAStartup(...)��һһ��Ӧ��ϵ

	return 0;
}