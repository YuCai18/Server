#include<WinSock2.h>
#include<Windows.h>
#include<stdio.h>

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
	//4.accept�ȴ����տͻ�������
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	char msgBuf[] = "Hello, I'm Server.";

	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("���󣬽��յ���Ч�ͻ���Socket...\n");
	}
	printf("�¿ͻ��˼���:IP = %s \n", inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true)
	{	
		//5.���տͻ�������
		int nLen = recv(_cSock,_recvBuf,128,0);
		if (nLen <= 0) {
			printf("�ͻ������˳������������");
			break;
		}
		//6.��������
		if (0 == strcmp(_recvBuf,"getName")) {
			//7.��ͻ��˷���һ������
			char msgBuf[] = "Cai Yu.";
			send(_cSock,msgBuf,strlen(msgBuf)+1,0);
		}
		else if (0 == strcmp(_recvBuf,"getAge")) {
			//7.��ͻ��˷���һ������
			char msgBuf[] = "24.";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else {
			//7.��ͻ��˷���һ������
			char msgBuf[] = "???.";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		
	}
	//8.�ر��׽���closesocket
	closesocket(_sock);

	WSACleanup();
	printf("�ͻ������˳������������");
	getchar();
	return 0;
}