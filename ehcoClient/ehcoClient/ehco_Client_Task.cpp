#include <iostream>
#include <WS2tcpip.h>
#include <conio.h>
#include <windows.h>
#include <string>
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable:4996)
using namespace std;
constexpr int SERVER_PORT = 3500;
string SERVER_IP;
const char* SERVER_IP_ADDR;
constexpr int BUF_SIZE = 1024;

void display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    cout << msg;
    wcout << lpMsgBuf << endl;
    LocalFree(lpMsgBuf);
}

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
            if (i == 0 && j == 0)
                cout << "■";
            else
                cout << "□";
        }
        cout << endl;
    }
}
int currentPosX = 0;
int currentPosY = 0;

struct Pos {
    int x = 0;
    int y = 0;
    char key;
};
#pragma pack()
Pos pos;
int main()
{
    char key = 0;
    cout << "Server IP: ";
    cin >> SERVER_IP;
    SERVER_IP_ADDR = SERVER_IP.c_str();
    system("cls");
    chessBoard(8, 8);
    setCursorPos(currentPosX, currentPosY);

    wcout.imbue(locale("korean"));
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP_ADDR, &server_addr.sin_addr);
    WSAConnect(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr), 0, 0, 0, 0);
    Pos pos;
    while (true)
    {
        char mess[BUF_SIZE];
        memset(&mess, 0, sizeof(mess));
        mess[0] = key;
        char r_mess[BUF_SIZE];
        memset(&r_mess, 0, sizeof(mess));
        if (kbhit()) {
            int pressedKey = getch();
            switch (pressedKey) {
            case 72:
            {
                //위
                pos.key = 116;
                WSABUF s_wsabuf[BUF_SIZE]; // 보낼 버퍼
                s_wsabuf[0].buf = (char*)&pos;
                s_wsabuf[0].len = sizeof(pos);
                DWORD bytes_sent;
                WSASend(server, s_wsabuf, 1, &bytes_sent, 0, NULL, NULL);

                WSABUF r_wsabuf[1]; // 받을버퍼 버퍼
                r_wsabuf[0].buf = r_mess;
                r_wsabuf[0].len = BUF_SIZE;
                DWORD bytes_recv;
                DWORD recv_flag = 0;
                int ret = WSARecv(server, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
                Pos* spos = (Pos*)&r_mess;
                pos.x = spos->x, pos.y = spos->y;
                setCursorPos(pos.x, pos.y+1);
                cout << "□";
                setCursorPos(pos.x, pos.y);
                cout << "■";
                break;
            }   
            case 80:
                //아래
                {
                    pos.key = 100;
                    WSABUF s_wsabuf[BUF_SIZE]; // 보낼 버퍼
                    s_wsabuf[0].buf = (char*)&pos;
                    s_wsabuf[0].len = sizeof(pos);
                    DWORD bytes_sent;
                    WSASend(server, s_wsabuf, 1, &bytes_sent, 0, NULL, NULL);

                    WSABUF r_wsabuf[1]; // 받을버퍼 버퍼
                    r_wsabuf[0].buf = r_mess;
                    r_wsabuf[0].len = BUF_SIZE;
                    DWORD bytes_recv;
                    DWORD recv_flag = 0;
                    int ret = WSARecv(server, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
                    Pos* spos = (Pos*)&r_mess;
                    pos.x = spos->x, pos.y = spos->y;
                    setCursorPos(pos.x, pos.y-1);
                    cout << "□";
                    setCursorPos(pos.x, pos.y);
                    cout << "■";
                }
                break;
            case 75:
                //왼쪽
                {
                    pos.key = 108;
                    WSABUF s_wsabuf[BUF_SIZE]; // 보낼 버퍼
                    s_wsabuf[0].buf = (char*)&pos;
                    s_wsabuf[0].len = sizeof(pos);
                    DWORD bytes_sent;
                    WSASend(server, s_wsabuf, 1, &bytes_sent, 0, NULL, NULL);

                    WSABUF r_wsabuf[1]; // 받을버퍼 버퍼
                    r_wsabuf[0].buf = r_mess;
                    r_wsabuf[0].len = BUF_SIZE;
                    DWORD bytes_recv;
                    DWORD recv_flag = 0;
                    int ret = WSARecv(server, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
                    Pos* spos = (Pos*)&r_mess;
                    pos.x = spos->x, pos.y = spos->y;
                    setCursorPos(pos.x+2, pos.y);
                    cout << "□";
                    setCursorPos(pos.x, pos.y);
                    cout << "■";

                }
                break;
            case 77:
                //오른쪽
                {
                
                    pos.key = 114;
                    WSABUF s_wsabuf[BUF_SIZE]; // 보낼 버퍼
                    s_wsabuf[0].buf = (char*)&pos;
                    s_wsabuf[0].len = sizeof(pos);
                    DWORD bytes_sent;
                    WSASend(server, s_wsabuf, 1, &bytes_sent, 0, NULL, NULL);

                    WSABUF r_wsabuf[1]; // 받을버퍼 버퍼
                    r_wsabuf[0].buf = r_mess;
                    r_wsabuf[0].len = BUF_SIZE;
                    DWORD bytes_recv;
                    DWORD recv_flag = 0;
                    int ret = WSARecv(server, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
                    Pos* spos = (Pos*)&r_mess;
                    pos.x = spos->x, pos.y = spos->y;
                    setCursorPos(pos.x-2, pos.y);
                    cout << "□";
                    setCursorPos(pos.x, pos.y);
                    cout << "■";
                   
                }
                break;
            default:
                break;
            }
        }
        //error
        //if (SOCKET_ERROR == ret) {
        //    display_error("recv error: ", WSAGetLastError());
        //    exit(-1);
        //}

    }
    closesocket(server);
    WSACleanup();
}
