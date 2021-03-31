#include <iostream>
#include <WS2tcpip.h>
#include <conio.h>
#include <windows.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996)
#define MAX_BUFFER        1024
#define SERVER_IP        "127.0.0.1"
#define SERVER_PORT        3500
#define MAX_CLIENT_COUNT 11

struct Player {
	int x = 0;
	int y = 0;
	char key = 0;
	int id = 0;
    int move=0;
};

void setCursorPos(int x, int y)
{
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void chessBoard(int x_line, int y_line)
{
    for (int i = 0; i < x_line; ++i)
    {
        for (int j = 0; j < y_line; ++j)
        {
            cout << "□";
        }
        cout << endl;
    }
}
int currentPosX = 0;
int currentPosY = 0;

int main()
{
    int c_id=1;

	WSADATA WSAData;
    u_long blockingMode = 0;
    u_long nonBlockingMode = 1;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
	connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    Player player[MAX_CLIENT_COUNT];
    chessBoard(8, 8);
    
    int sendBytes;
    int bufferLen = static_cast<int>(sizeof(player));
    int receiveBytes;
    

    
    setCursorPos(player[c_id].x, player[c_id].y);
    cout << "■";
    
    while (true) {
        
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i)
        {
            ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
            receiveBytes = recv(serverSocket, (char*)&player[i], sizeof(player), 0);
            if (player[i].id != i)
            {
                c_id = player[i - 1].id;
                break;
            }
        }
        //for (int i = 1; i < MAX_CLIENT_COUNT; ++i)
        //{
        //    if (player[c_id].id != player[i].id && player[i].id > 0) {
        //        //setCursorPos(player[i].x, player[i].y);
        //        //cout << "□";
        //        ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
        //        receiveBytes = recv(serverSocket, (char*)&player[i], sizeof(player), 0);
        //        setCursorPos(player[i].x, player[i].y);
        //        cout << "★";
        //    }
        //
        //}
        player[c_id].id = c_id;
        sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
        bufferLen = static_cast<int>(sizeof(player));
        if (kbhit()) {
            int pressedKey = getch();
            switch (pressedKey) {
            case 72:
            {
                //위
                player[c_id].key = 116;
                player[c_id].id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
                break;
            }
            case 80:
                //아래
            {
                player[c_id].key = 100;
                player[c_id].id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
                //setCursorPos(player[c_id].x, player[c_id].y);
                //cout << "□";
                //ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
                //receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
                //setCursorPos(player[c_id].x, player[c_id].y);
                //cout << "■";
                
            }
            break;
            case 75:
                //왼쪽
            {
               player[c_id].key = 108;
               player[c_id].id = c_id;
               sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
               bufferLen = static_cast<int>(sizeof(player));
               //setCursorPos(player[c_id].x, player[c_id].y);
               //cout << "□";
               //ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
               //receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
               //setCursorPos(player[c_id].x, player[c_id].y);
               //cout << "■";
              
        
            }
            break;
            case 77:
                //오른쪽
            {
        
                player[c_id].key = 114;
                player[c_id].id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
                //setCursorPos(player[c_id].x, player[c_id].y);
                //cout << "□";
                //ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
                //receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
                //setCursorPos(player[c_id].x, player[c_id].y);
                //    cout << "■";
            }
            break;
            default:
                break;
            }   
           
            
        }
  
        setCursorPos(player[c_id].x, player[c_id].y);
        cout << "□";
        for (int i = 0; i < MAX_CLIENT_COUNT; ++i)
        {
            ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
            receiveBytes = recv(serverSocket, (char*)&player[i], sizeof(player), 0);
            
            if (player[c_id].id == player[i].id)
            {
                setCursorPos(player[c_id].x, player[c_id].y);
                cout << "■";
            }

        }
        setCursorPos(player[c_id].x, player[c_id].y);
        cout << "□";
        for (int i = 1; i < MAX_CLIENT_COUNT; ++i)
        {
            ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
            receiveBytes = recv(serverSocket, (char*)&player[i], sizeof(player), 0);
            if (player[c_id].id != player[i].id && player[i].id > 0)
            {
                setCursorPos(player[i].x, player[i].y);
                cout << "★";
            }
        }

      
        //해야하는것 id어떻게 구분해서 어떻게 클라에게 보낼것인지
        //send recv 반복돌려야하는건지
        //10명 접속 어떻게 구분할것인지
       
        
        if (bufferLen == 0) break;
	}
	closesocket(serverSocket);
	WSACleanup();
}
