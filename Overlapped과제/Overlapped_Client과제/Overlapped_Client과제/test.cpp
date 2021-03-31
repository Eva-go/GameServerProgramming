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
#define MAX_CLIENT_COUNT 10
struct Player {
    int x = 0;
    int y = 0;
    char key = 0;
    int id = 0;
    char playerGUI;
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
    int c_id;
    u_long blockingMode = 0;
    u_long nonBlockingMode = 1;
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
    connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    Player player;
    chessBoard(8, 8);

    int sendBytes;
    int bufferLen = static_cast<int>(sizeof(player));
    int receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
    c_id = player.id;
    setCursorPos(player.x, player.y);
    cout << "■";
    if (player.id != c_id)
    {
        setCursorPos(player.x, player.y);
        cout << "★";
    }
    while (true) {
        if (kbhit()) {
            int pressedKey = getch();
            switch (pressedKey) {
            case 72:
            {
                //위
                player.key = 116;
                player.id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
                
                break;
            }
            case 80:
                //아래
            {
                player.key = 100;
                player.id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
               
            }
            break;
            case 75:
                //왼쪽
            {
                player.key = 108;
                player.id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
               

            }
            break;
            case 77:
                //오른쪽
            {

                player.key = 114;
                player.id = c_id;
                sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
                bufferLen = static_cast<int>(sizeof(player));
               
            }
            break;
            default:
                break;
            }
            setCursorPos(player.x, player.y);
            cout << "□";
            ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
            receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
            setCursorPos(player.x, player.y);
            cout << "■";
        }
        //해야하는것 id어떻게 구분해서 어떻게 클라에게 보낼것인지
        //send recv 반복돌려야하는건지
        //10명 접속 어떻게 구분할것인지
        //sendBytes = send(serverSocket, (char*)&player, sizeof(player), 0);
        //bufferLen = static_cast<int>(sizeof(player));
        //receiveBytes = recv(serverSocket, (char*)&player, sizeof(player), 0);
        //if (player.id != c_id)
        //{
        //    setCursorPos(player.x - 2, player.y);
        //    cout << "□";
        //    setCursorPos(player.x, player.y);
        //    cout << "★";
        //}
        if (bufferLen == 0) break;
    }
    closesocket(serverSocket);
    WSACleanup();
}