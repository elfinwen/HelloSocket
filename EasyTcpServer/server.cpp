
#include "EasyTcpServer.hpp"



int main()
{
	EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr, 4567);
	server1.Listen(5);

	while (server1.isRun())
	{
		server1.OnRun();
	}
	server1.Close();
	printf("���˳������������\n");
	getchar();

	return 0;
}