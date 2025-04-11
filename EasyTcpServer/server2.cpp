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
//��Ϣͷ
struct DataHeader
{
	short dataLength; //���ݳ���
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
	//������
	char szRecv[4096] = {};
	//5.���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("�ͻ���<Socket=%d>���˳������������\n",_cSock);
		return -1;
	}
	switch (header->cmd)
	{
		case CMD_LOGIN:
		{

			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("�յ��ͻ���<Socket=%d>,����:CMD_LOGIN ���ݳ���:%d ,userName = %s,PassWord = %s\n",_cSock,login->dataLength, login->userName, login->PassWord);
			//�����ж��û��������Ƿ���ȷ
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
		}
		return 1;

		case CMD_LOGOUT:
		{

			recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Logout* logout = (Logout*)szRecv;
			printf("�յ��ͻ���<Socket=%d>,����:CMD_LOGOUT ���ݳ���:%d,userName = %s\n",_cSock,logout->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ
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
	//1.����һ���׽���
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//2.bind�󶨽��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR,�󶨶˿�ʧ�ܣ�\n");
	}
	else {
		printf("�󶨶˿ڳɹ���\n");
	}
	//3.listen��������˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("ERROR,�����˿�ʧ�ܣ�\n");
	}
	else {
		printf("�����˿ڳɹ���\n");
	}

	while (true)
	{
		//������ socket ������
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		FD_ZERO(&fdRead); //��������������
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		// �����пͻ����׽��ּ��������
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		//ndfs��һ������ֵ����ָfd_set�����У�����������(socket)�ķ�Χ��������������
		//���������ļ����������ֵ+1����windows�������������ν������д0��
		//�����ȴ��¼�������ֱ�����׽��ֿɶ�����д�����쳣��
		timeval t = {1,0}; //��ȴ�һ��ʱ��

		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("select���������\n");
			break;
		}
		//���������ӣ������׽��ֿɶ���
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead); //�Ӽ������Ƴ�����ѡ��
			//4.accept�ȴ����տͻ�������
			sockaddr_in clientAddr = {}; 
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			char msgBuf[] = "Hello, I'm Server.";

			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); // ����������
			if (INVALID_SOCKET == _cSock) {
				printf("���󣬽��յ���Ч�ͻ���Socket...\n");
			}
			else 
			{
				//Ⱥ��
				for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
					NewUserJoin userJoin;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
			}

			g_clients.push_back(_cSock);
			printf("�¿ͻ��˼���:socket = %d,IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

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
		printf("����ʱ�䴦������ҵ��...\n");
	}
	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]); //�ر����пͻ����������ͷ���Դ��
	}
	//8.�ر��׽���closesocket
	closesocket(_sock);

	WSACleanup();
	printf("�ͻ������˳������������\n");
	getchar();
	return 0;
}