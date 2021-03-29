#include <iostream>
#include <map>
using namespace std;
#include <WS2tcpip.h>
#include <random>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_PORT        3500
#define MAX_CLIENT_COUNT 10

uniform_int_distribution<int> un(0, 7);
normal_distribution<> nd{ 0.0,1.0 };
default_random_engine dre{ random_device()() };

struct Player {
	int x = 0;
	int y = 0;
	char key = 0;
	int id = 0;
	char playerGUI;
};
Player player;
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	char messageBuffer[MAX_BUFFER];
};

map <SOCKET, SOCKETINFO> clients;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<SOCKETINFO*>(overlapped)->socket;

	if ('t' == player.key && player.y > 0) {
		player.y -= 1;
	}
	else if ('d' == player.key && player.y < 7) {
		player.y += 1;
	}
	else if ('l' == player.key && player.x > 0) {
		player.x -= 2;
	}
	else if ('r' == player.key && player.x < 14) {
		player.x += 2;
	}
	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���
	clients[client_s].dataBuffer.len = sizeof(player);
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	

	clients[client_s].dataBuffer.buf = (char*)&player;


	WSASend(client_s, &clients[client_s].dataBuffer, 1, NULL, 0, overlapped, send_callback);
	cout << "send" << endl;
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	
	SOCKET client_s = reinterpret_cast<SOCKETINFO*>(overlapped)->socket;
	if (dataBytes == 0) {
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	// WSASend(���信 ����)�� �ݹ��� ���
	cout << "recv " << endl;
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	clients[client_s].dataBuffer.len = MAX_BUFFER;
	DWORD flags = 0;
	
	WSARecv(client_s, &clients[client_s].dataBuffer, 1, 0, &flags, overlapped, recv_callback);
}

int number;

int main()
{
	
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	//���⼭ ���̵�� ������ǥ �����
	
	listen(listenSocket, 5);
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);

	while (true) {
		SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
		player.id += 1;
		player.x = un(dre);
		player.x = player.x * 2;
		player.y = un(dre);
		clients[clientSocket] = SOCKETINFO{};
		clients[clientSocket].socket = clientSocket;
		clients[clientSocket].dataBuffer.len = sizeof(player);
		clients[clientSocket].dataBuffer.buf = (char*)&player;
		memset(&clients[clientSocket].overlapped, 0, sizeof(WSAOVERLAPPED));
		//clients[clientSocket].overlapped.hEvent = (HANDLE)clients[clientSocket].socket;

		DWORD flags = 0;
		//�̵�ó��
		
		WSASend(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, 0, &(clients[clientSocket].overlapped), send_callback);
		cout << player.x<<"," << player.y << endl;
		//WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, &flags, &(clients[clientSocket].overlapped), recv_callback);

	}
	closesocket(listenSocket);
	WSACleanup();
}

