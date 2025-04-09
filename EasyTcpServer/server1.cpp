#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

struct DataPackage
{
	int age;
	char name[32];
};

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;

	WSAStartup(ver, &dat);
	//1.建立一个套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//2.bind绑定接收客户端链接的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR,绑定端口失败！\n");
	}
	else {
		printf("绑定端口成功！\n");
	}
	//3.listen监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("ERROR,监听端口失败！\n");
	}
	else {
		printf("监听端口成功！\n");
	}
	//4.accept等待接收客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	char msgBuf[] = "Hello, I'm Server.";

	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("错误，接收到无效客户端Socket...\n");
	}
	printf("新客户端加入:IP = %s \n", inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true)
	{	
		//5.接收客户端数据
		int nLen = recv(_cSock,_recvBuf,128,0);
		if (nLen <= 0) {
			printf("客户端已退出，任务结束。");
			break;
		}
		printf("收到命令:%s \n",_recvBuf);
		//6.处理请求
		if (0 == strcmp(_recvBuf, "getInfo")) {
			DataPackage dp = {24,"Cai Yu"};
			send(_cSock, (const char*) & dp, sizeof(DataPackage), 0);
		}
		else {
			//7.向客户端发送一条数据
			char msgBuf[] = "???.";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		
	}
	//8.关闭套接字closesocket
	closesocket(_sock);

	WSACleanup();
	printf("客户端已退出，任务结束。");
	getchar();
	return 0;
}