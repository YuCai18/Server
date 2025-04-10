#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};
//消息头
struct DataHeader
{
	short dataLength; //数据长度
	short cmd;
};
//DataPackage
struct Login:public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};

struct LoginResult:public DataHeader
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

	
	while (true)
	{	
		DataHeader header = {};
		//5.接收客户端数据
		int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
		if (nLen <= 0) {
			printf("客户端已退出，任务结束。");
			break;
		}
		
		switch (header.cmd)
		{
			case CMD_LOGIN:
				{
					Login login = {};
					recv(_cSock,(char*)&login + sizeof(DataHeader),sizeof(Login) - sizeof(DataHeader),0);
					printf("收到命令:CMD_LOGIN 数据长度:%d ,userName = %s,PassWord = %s\n",login.dataLength,login.userName,login.PassWord);
					//忽略判断用户名密码是否正确
					LoginResult ret;
					send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
				}
				break;

			case CMD_LOGOUT:
				{
					Logout logout = {};
					recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(logout) - sizeof(DataHeader), 0);
					printf("收到命令:CMD_LOGOUT 数据长度:%d,userName = %s\n", logout.dataLength, logout.userName);
					//忽略判断用户名密码是否正确
					LogoutResult ret;
					send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
				}
				break;
			default:
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);

				break;
		}

		
	}
	//8.关闭套接字closesocket
	closesocket(_sock);

	WSACleanup();
	printf("客户端已退出，任务结束。");
	getchar();
	return 0;
}