#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR        (-1)
	#define closesocket(s) close(s)
#endif
#include<stdio.h>
#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}

	~EasyTcpServer()
	{
		Close();
	}
	//初始化Socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket<%d>失败...\n",_sock);
		}
		else {
			printf("建立Socket=<%d>成功...\n", _sock);
		}
	}
	//绑定IP和端口
	int Bind(const char *ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); // host to net unsigned short
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("ERROR,绑定端口<%d>失败！\n",port);
		}
		else {
			printf("绑定端口<%d>成功！\n",port);
		}
		return ret;
	}
	//监听端口
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("Socket=<%d>错误,监听端口失败！\n",_sock);
		}
		else {
			printf("Socket=<%d>监听端口成功！\n",_sock);
		}
		return ret;
	}
	//接收客户端链接
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); // 接受新连接
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen); // 接受新连接
#endif
		if (INVALID_SOCKET == _cSock) {
			printf("socket=<%d>错误，接收到无效客户端Socket...\n",_sock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("socket=<%d>新客户端加入:socket = %d,IP = %s \n", _sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _cSock; 
	}
	//关闭Socket 
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]); //关闭所有客户端连接以释放资源。
			}
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]); //关闭所有客户端连接以释放资源。
			}
			close(_sock);
#endif
		}

	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
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
			SOCKET maxSock = _sock;
			// 将所有客户端套接字加入读集合
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n]) {
					maxSock = g_clients[n];
				}
			}

			//ndfs是一个整数值，是指fd_set集合中，所有描述符(socket)的范围，而不是数量。
			//即是所有文件描述符最大值+1，在windows中这个参数无所谓，可以写0。
			//阻塞等待事件发生，直到有套接字可读、可写或发生异常。
			timeval t = { 1,0 }; //最长等待一秒时间

			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0) {
				printf("select任务结束。\n");
				Close();
				return false;
			}
			//处理新连接（监听套接字可读）
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead); //从集合中移除
				Accept();

			}
			for (int n = (int)g_clients.size() - 1; n >= 0; --n) {
				if (FD_ISSET(g_clients[n], &fdRead)) {
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin();
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			printf("空闲时间处理其他业务...\n");
			return true;
		}
		return false;

	}
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[4096] = {};
		//5.接收客户端数据
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("客户端<Socket=%d>已退出，任务结束。\n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("收到客户端<Socket=%d>,请求:CMD_LOGIN 数据长度:%d ,userName = %s,PassWord = %s\n", 
				_cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户名密码是否正确
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;

		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("收到客户端<Socket=%d>,命令:CMD_LOGOUT 数据长度:%d,userName = %s\n", _cSock, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确
			LogoutResult ret;
			send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
		}
		break;
		default:
		{
			DataHeader header = {0,CMD_ERROR};
			send(_cSock, (char*)&header, sizeof(header), 0);
		}

		break;
		}
	}
	//发送指定Socket数据
	int SendData(SOCKET _cSock,DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, sizeof(header), 0);
		}
		return SOCKET_ERROR;
	}

	//群发数据
	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			SendData(g_clients[n], header);
		}
	}
};



#endif // !_EasyTcpServer_hpp_

