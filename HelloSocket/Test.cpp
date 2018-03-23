#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")


int main()
{
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);//版本号，对应上面的头文件Winsock2.h  
	WSADATA dat;
	WSAStartup(ver, &dat);//与WSACleanup()是一一对应关系
	//----------------以下是socket网络通信部分
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	// 2 连接服务器 connect
	// 3 接收服务器信息 recv
	// 4 关闭套接字closesocket
	//-- 用Socket API建立简易TCP服务端
	// 1 建立一个socket
	// 2 bind 绑定用于接受客户端连接的网络端口
	// 3 listen 监听网络端口
	// 4 accept 等待接收客户端连接
	// 5 send 向客户端发送一条数据
	// 6 关闭套接字closesocket


	//----------------以上是socket网络通信部分
	//清除Windows socket环境
	WSACleanup();//与WSAStartup(...)是一一对应关系

	return 0;
}