#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>
#include"vector"


enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
//消息头
struct DataHeader
{
	short dataLength; //数据长度
	short cmd;
};
//DataPackage
struct Login :public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult :public DataHeader
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
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

std::vector<SOCKET> g_clients;
int processor(SOCKET _cSock)
{	
	//缓冲区
	char szRecv[4096] = {};
	//5.接收客户端数据
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("客户端<Socket=%d>已退出，任务结束。\n",_cSock);
		return -1;
	}
	switch (header->cmd)
	{
		case CMD_LOGIN:
		{

			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("收到客户端<Socket=%d>,请求:CMD_LOGIN 数据长度:%d ,userName = %s,PassWord = %s\n",_cSock,login->dataLength, login->userName, login->PassWord);
			//忽略判断用户名密码是否正确
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		return 1;

		case CMD_LOGOUT:
		{

			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Logout* logout = (Logout*)szRecv;
			printf("收到客户端<Socket=%d>,命令:CMD_LOGOUT 数据长度:%d,userName = %s\n",_cSock,logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确
			LogoutResult ret;
			send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		return 1;
		default:
		{
			DataHeader header = { 0,CMD_ERROR };
			send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		}

		return 0;
	}

}

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

	while (true)
	{
		//伯克利 socket 描述符
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		FD_ZERO(&fdRead); //把这三个结果清空
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		// 将所有客户端套接字加入读集合
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		//ndfs是一个整数值，是指fd_set集合中，所有描述符(socket)的范围，而不是数量。
		//即是所有文件描述符最大值+1，在windows中这个参数无所谓，可以写0。
		//阻塞等待事件发生，直到有套接字可读、可写或发生异常。
		timeval t = {1,0}; //最长等待一秒时间

		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("select任务结束。\n");
			break;
		}
		//处理新连接（监听套接字可读）
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead); //从集合中移除（可选）
			//4.accept等待接收客户端连接
			sockaddr_in clientAddr = {}; 
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			char msgBuf[] = "Hello, I'm Server.";

			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); // 接受新连接
			if (INVALID_SOCKET == _cSock) {
				printf("错误，接收到无效客户端Socket...\n");
			}
			else 
			{
				//群发
				for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
			}

			g_clients.push_back(_cSock);
			printf("新客户端加入:socket = %d,IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

		}
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(),g_clients.end(),fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
		printf("空闲时间处理其他业务...\n");
	}
	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]); //关闭所有客户端连接以释放资源。
	}
	//8.关闭套接字closesocket
	closesocket(_sock);

	WSACleanup();
	printf("客户端已退出，任务结束。\n");
	getchar();
	return 0;
}