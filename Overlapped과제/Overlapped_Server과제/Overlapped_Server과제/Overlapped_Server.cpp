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

void display_error(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

struct Player {
	int x = 0;
	int y = 0;
	char key = 0;
	int id = 0;
	int move = 0;
};
Player player[MAX_CLIENT_COUNT];
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	char messageBuffer[MAX_BUFFER];
};
int s_id = 0;
map <SOCKET, SOCKETINFO> clients;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<SOCKETINFO*>(overlapped)->socket;

	for (int i = 0; i < MAX_CLIENT_COUNT; ++i)
	{
		if (player[i].id == i+1 && 't' == player[i].key && player[i].y > 0) {
			player[i].y -= 1;
		
		}
		else if (player[i].id == i + 1 && 'd' == player[i].key && player[i].y < 7) {
			player[i].y += 1;
			
		}
		else if (player[i].id == i + 1 && 'l' == player[i].key && player[i].x > 0) {
			player[i].x -= 2;
			
		}
		else if (player[i].id == i + 1 && 'r' == player[i].key && player[i].x < 14) {
			player[i].x += 2;
			
		}
		
	}
	

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우
	clients[client_s].dataBuffer.len = sizeof(player);
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	clients[client_s].dataBuffer.buf = (char*)&player;


	auto s_send = WSASend(client_s, &clients[client_s].dataBuffer, 1, NULL, 0, overlapped, send_callback);
	if (s_send == SOCKET_ERROR) {
		cout << "ERROR!: " << endl;
		display_error("Send error: ", WSAGetLastError());
	}
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	
	SOCKET client_s = reinterpret_cast<SOCKETINFO*>(overlapped)->socket;
	if (dataBytes == 0) {
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

	// WSASend(응답에 대한)의 콜백일 경우
	clients[client_s].dataBuffer.len = sizeof(player);
	memset(&(clients[client_s].overlapped), 0, sizeof(WSAOVERLAPPED));
	DWORD flags = 0;
	cout << player[0].x << endl;

	
	auto result=WSARecv(client_s, &clients[client_s].dataBuffer, 1, 0, &flags, overlapped, recv_callback);
	cout << player[0].x << endl;
	if (result == SOCKET_ERROR) {
		cout << "ERROR!: " << endl;
		display_error("Recv error: ", WSAGetLastError());
	}
}

int number;

int main()
{

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	wcout.imbue(locale("korean"));
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));

	listen(listenSocket, 5);
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);


	while (true) {
		SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
		
		player[s_id].id +=1 ;
		player[s_id].x = un(dre);
		player[s_id].x = player[s_id].x * 2;
		player[s_id].y = un(dre);
		s_id += 1;

		
		clients[clientSocket] = SOCKETINFO{};
		clients[clientSocket].socket = clientSocket;
		clients[clientSocket].dataBuffer.len = sizeof(player);
		clients[clientSocket].dataBuffer.buf = (char*)&player;
		memset(&clients[clientSocket].overlapped, 0, sizeof(WSAOVERLAPPED));
		//clients[clientSocket].overlapped.hEvent = (HANDLE)clients[clientSocket].socket;

		DWORD flags = 0;
		//이동처리
		
		WSASend(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, 0, &(clients[clientSocket].overlapped), send_callback);
		//WSARecv(clients[clientSocket].socket, &clients[clientSocket].dataBuffer, 1, NULL, &flags, &(clients[clientSocket].overlapped), recv_callback);

	}
	closesocket(listenSocket);
	WSACleanup();
}

