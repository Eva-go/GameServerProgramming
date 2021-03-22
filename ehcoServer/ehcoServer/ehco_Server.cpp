#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int SERVER_PORT = 3500;
const char* SERVER_IP = "127.0.0.1"; // 자기 자신의 주소는 항상 127.0.0.1
constexpr int BUF_SIZE = 1024;

struct Pos {
    int x = 0;
    int y = 0;
    char key=0;
};
#pragma pack()
Pos pos;

int main()
{
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, 0);
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;


    //bind
    bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    //listen
    listen(server, SOMAXCONN);
    SOCKADDR_IN cl_addr;
    int addr_size = sizeof(cl_addr);

    //Accept

    SOCKET client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, NULL, NULL);
    while (true)
    {
        // recv
        char r_mess[BUF_SIZE];
        WSABUF r_wsabuf[1]; // 받을 버퍼
        r_wsabuf[0].buf = r_mess;
        r_wsabuf[0].len = BUF_SIZE;
        DWORD bytes_recv;
        DWORD recv_flag = 0;
        WSARecv(client, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
        Pos*cpos = (Pos*)&r_mess;
        pos.x = cpos->x, pos.y = cpos->y, pos.key = cpos->key;
        if ('t' == pos.key&&pos.y>0) {
            pos.y -= 1;
        }
        if ('d' == pos.key&& pos.y < 7) {
            pos.y += 1;
        }
        if ('l' == pos.key&& pos.x > 0) {
            pos.x -= 2;
        }
        if ('r' == pos.key&& pos.x < 14) {
            pos.x += 2;
        }
        // send
        WSABUF s_wsabuf[BUF_SIZE]; // 보낼 버퍼
        s_wsabuf[0].buf = (char*)&pos;
        s_wsabuf[0].len = sizeof(pos);
        DWORD bytes_sent;
        WSASend(client, s_wsabuf, 1, &bytes_sent, 0, NULL, NULL);
    }
    closesocket(server);
    WSACleanup();
}
