#include <iostream>
#include <unordered_set>
#include <thread>
#include <vector>
#include <mutex>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#include "protocol.h"
#include <queue>
using namespace std;
enum OP_TYPE { OP_RECV, OP_SEND, OP_ACCEPT,OP_RANDOM_MOVE,OP_ATTACK};
struct EX_OVER
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsabuf[1];
	unsigned char	m_packetbuf[MAX_BUFFER];
	OP_TYPE			m_op;
	SOCKET			m_csocket;					// OP_ACCEPT에서만 사용
};
enum PL_STATE { PLST_FREE, PLST_CONNECTED, PLST_INGAME };

struct TIMER_EVENT {
	int object;
	OP_TYPE e_type;
	chrono::system_clock::time_point start_time;
	int target_id;

	constexpr bool operator< (const TIMER_EVENT& L) const {
		return (start_time > L.start_time);
	}
};

priority_queue <TIMER_EVENT> timer_queue;
mutex timer_l;

struct S_OBJECT
{
	mutex  m_slock;
	atomic <PL_STATE> m_state;
	SOCKET m_socket;
	int		id;
	EX_OVER m_recv_over;
	int m_prev_size;
	atomic_bool is_active;

	char m_name[200];
	short	x, y;
	int move_time;
	unordered_set<int> m_view_list;
	mutex m_vl;
};

constexpr int SERVER_ID = 0;
array <S_OBJECT, MAX_USER + 1> object;
HANDLE h_iocp;

void add_event(int obj, OP_TYPE ev_t, int delay_ms)
{
	using namespace chrono;
	TIMER_EVENT ev;
	ev.e_type = ev_t;
	ev.object = obj;
	ev.start_time = system_clock::now() + milliseconds(delay_ms);
	timer_l.lock();
	timer_queue.push(ev);
	timer_l.unlock();
}

void display_error(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

bool is_npc(int id)
{
	return id>NPC_START;
}

bool can_see(int id_a, int id_b)
{
	return VIEW_RADIUS >= abs(object[id_a].x - object[id_b].x) && //2차원 거리 계산
		VIEW_RADIUS >= abs(object[id_a].y - object[id_b].y);

}

void process_packet(int p_id, unsigned char* p_buf);

void disconnect(int p_id);

void send_packet(int p_id, void* p)
{
	int p_size = reinterpret_cast<unsigned char*>(p)[0];
	int p_type = reinterpret_cast<unsigned char*>(p)[1];
	//cout << "To client [" << p_id << "] : ";
	//cout << "Packet [" << p_type << "]\n";
	EX_OVER* s_over = new EX_OVER;
	s_over->m_op = OP_SEND;
	memset(&s_over->m_over, 0, sizeof(s_over->m_over));
	memcpy(s_over->m_packetbuf, p, p_size);
	s_over->m_wsabuf[0].buf = reinterpret_cast<CHAR*>(s_over->m_packetbuf);
	s_over->m_wsabuf[0].len = p_size;
	int ret = WSASend(object[p_id].m_socket, s_over->m_wsabuf, 1,
		NULL, 0, &s_over->m_over, 0);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) {
			display_error("WSASend : ", WSAGetLastError());
			disconnect(p_id);
		}

	}
}
void do_recv(int key)
{
	object[key].m_recv_over.m_wsabuf[0].buf =
		reinterpret_cast<char*>(object[key].m_recv_over.m_packetbuf)
		+ object[key].m_prev_size;
	object[key].m_recv_over.m_wsabuf[0].len = MAX_BUFFER - object[key].m_prev_size;
	memset(&object[key].m_recv_over.m_over, 0, sizeof(object[key].m_recv_over.m_over));
	DWORD r_flag = 0;
	int ret = WSARecv(object[key].m_socket, object[key].m_recv_over.m_wsabuf, 1,
		NULL, &r_flag, &object[key].m_recv_over.m_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("WSARecv : ", WSAGetLastError());
	}
}
int get_new_player_id(SOCKET p_socket)
{
	for (int i = SERVER_ID + 1; i <= MAX_USER; ++i) {
		lock_guard<mutex> lg{ object[i].m_slock };
		if (PLST_FREE == object[i].m_state) {
			object[i].m_state = PLST_CONNECTED;
			object[i].m_socket = p_socket;
			object[i].m_name[0] = 0;
			return i;
		}
	}
	return -1;
}
void send_login_ok_packet(int p_id)
{
	s2c_login_ok p;
	p.hp = 10;
	p.id = p_id;
	p.level = 2;
	p.race = 1;
	p.size = sizeof(p);
	p.type = S2C_LOGIN_OK;
	p.x = object[p_id].x;
	p.y = object[p_id].y;
	send_packet(p_id, &p);
}
void send_move_packet(int c_id, int p_id)
{
	s2c_move_player p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = S2C_MOVE_PLAYER;
	p.x = object[p_id].x;
	p.y = object[p_id].y;
	p.move_time = object[p_id].move_time;
	send_packet(c_id, &p);
}
void send_add_object(int c_id, int p_id)
{
	s2c_add_player p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = S2C_ADD_PLAYER;
	p.x = object[p_id].x;
	p.y = object[p_id].y;
	p.race = 0;
	strcpy_s(p.name, object[p_id].m_name);
	send_packet(c_id, &p);
}
void send_remove_object(int c_id, int p_id)
{
	s2c_remove_player p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = S2C_REMOVE_PLAYER;
	send_packet(c_id, &p);
}

void wake_up_npc(int npc_id) 
{
	if (object[npc_id].is_active == false) {
		bool old_state = false;
		if(true == atomic_compare_exchange_strong(&object[npc_id].is_active, &old_state,true))
			add_event(npc_id, OP_RANDOM_MOVE, 1000);
	}
}

void do_move(int p_id, DIRECTION dir)
{
	auto& x = object[p_id].x;
	auto& y = object[p_id].y;

	switch (dir) {
	case D_N: if (y > 0) y--; break;
	case D_S: if (y < (WORLD_Y_SIZE - 1)) y++; break;
	case D_W: if (x > 0) x--; break;
	case D_E: if (x < (WORLD_X_SIZE - 1)) x++; break;
	}

	unordered_set<int> old_vl;
	object[p_id].m_vl.lock();
	old_vl = object[p_id].m_view_list;
	object[p_id].m_vl.unlock();
	unordered_set<int> new_vl;
	for (auto& pl : object) {
		if (pl.id == p_id) continue;
		if ((pl.m_state == PLST_INGAME) && can_see(p_id, pl.id)) {
			new_vl.insert(pl.id);
		}
	}
	send_move_packet(p_id, p_id);
	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {		//1. 새로 시야에 들어오는 객체가 있으면
			object[p_id].m_vl.lock();
			object[p_id].m_view_list.insert(pl);
			object[p_id].m_vl.unlock();
			send_add_object(p_id, pl);

			if (false == is_npc(pl)) {
				object[pl].m_vl.lock();
				if (0 == object[pl].m_view_list.count(p_id)) {
					object[pl].m_view_list.insert(p_id);
					object[pl].m_vl.unlock();
					send_add_object(pl, p_id);
				}
				else {
					object[pl].m_vl.unlock();
					send_move_packet(pl, p_id);
				}
			}
			else wake_up_npc(pl);
		}
		else { //2. 기존시야에도 있고 새 시야에도 있는 경우
			if (false == is_npc(pl)) {
				object[pl].m_vl.lock();
				if (0 == object[pl].m_view_list.count(p_id)) {
					object[pl].m_view_list.insert(p_id);
					object[pl].m_vl.unlock();
					send_add_object(pl, p_id);
				}
				else {
					object[pl].m_vl.unlock();
					send_move_packet(pl, p_id);
				}
			}
		}
	}

	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			//3. 시야에서 사라진 경우
			object[p_id].m_vl.lock();
			object[p_id].m_view_list.erase(pl);
			object[p_id].m_vl.unlock();
			send_remove_object(p_id, pl);

			if (false == is_npc(pl)) {
				object[pl].m_vl.lock();
				if (0 != object[pl].m_view_list.count(p_id)) {
					object[pl].m_view_list.erase(p_id);
					object[pl].m_vl.unlock();
					send_remove_object(pl, p_id);
				}
				else {
					object[pl].m_vl.unlock();
				}
			}
		}
	}
}

void disconnect(int p_id)
{
	{
		lock_guard <mutex> gl{ object[p_id].m_slock };
		if (PLST_FREE == object[p_id].m_state) return;
		closesocket(object[p_id].m_socket);
		object[p_id].m_state = PLST_FREE;
	}
	for (auto& pl : object) {
		if (false == is_npc(pl.id)) {
			lock_guard<mutex> gl2{ pl.m_slock };
			if (PLST_INGAME == pl.m_state)
				send_remove_object(pl.id, p_id);
		}
	}
}

void do_npc_random_move(S_OBJECT& npc)
{
	unordered_set<int> old_vl;
	for (auto& obj : object) {
		if (PLST_INGAME != obj.m_state) continue;
		if (true == is_npc(obj.id)) continue;
		if (true == can_see(npc.id, obj.id))
			old_vl.insert(obj.id);
	}
	int x = npc.x;
	int y = npc.y;
	switch (rand() % 4)
	{
	case 0: if (x < (WORLD_X_SIZE - 1))x++; break;
	case 1: if (x > (0))x--; break;
	case 2: if (y < (WORLD_Y_SIZE - 1))y++; break;
	case 3: if (y > 0)y--; break;
	}
	npc.x = x;
	npc.y = y;

	unordered_set<int> new_vl;
	for (auto& obj : object) {
		if (PLST_INGAME != obj.m_state) continue;
		if (true == is_npc(obj.id)) continue;
		if (true == can_see(npc.id, obj.id))
			new_vl.insert(obj.id);
	}

	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			//플레이어의 시야에 등장
			object[pl].m_vl.lock();
			object[pl].m_view_list.insert(npc.id);
			object[pl].m_vl.unlock();
			send_add_object(pl, npc.id);
		}
		else {
			//플레이어가 계속 보고 있음
			send_move_packet(pl, npc.id);
		}
	}
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			object[pl].m_vl.lock();
			if (0 != object[pl].m_view_list.count(pl)) {
				object[pl].m_view_list.erase(pl);
				object[pl].m_vl.unlock();
				send_remove_object(pl, npc.id);
			}
			else {
				object[pl].m_vl.unlock();
			}
		}
	}
}


void process_packet(int p_id, unsigned char* p_buf)
{
	switch (p_buf[1]) {
	case C2S_LOGIN: {
		c2s_login* packet = reinterpret_cast<c2s_login*>(p_buf);
		lock_guard <mutex> gl2{ object[p_id].m_slock };
		strcpy_s(object[p_id].m_name, packet->name);
		object[p_id].x = rand() % WORLD_X_SIZE;
		object[p_id].y = rand() % WORLD_Y_SIZE;
		send_login_ok_packet(p_id);
		object[p_id].m_state = PLST_INGAME;

		for (auto& pl : object) {
			if (p_id != pl.id) {
				lock_guard <mutex> gl{ pl.m_slock };
				if (PLST_INGAME == pl.m_state) {
					if (can_see(p_id, pl.id)) {
						object[p_id].m_vl.lock();
						object[p_id].m_view_list.insert(pl.id);
						object[p_id].m_vl.unlock();
						send_add_object(p_id, pl.id);
						if (false == is_npc(pl.id)) {
							object[pl.id].m_vl.lock();
							object[pl.id].m_view_list.insert(p_id);
							object[pl.id].m_vl.unlock();
							send_add_object(pl.id, p_id);
						}
						else {
							wake_up_npc(pl.id);
						}
					}
				}
			}
		}
	}
				  break;
	case C2S_MOVE: {
		c2s_move* packet = reinterpret_cast<c2s_move*>(p_buf);
		object[p_id].move_time = packet->move_time;
		do_move(p_id, packet->dr);
	}
				 break;
	default:
		//cout << "Unknown Packet Type from Client[" << p_id;
		//cout << "] Packet Type [" << p_buf[1] << "]";
		while (true);
	}
}

void worker(HANDLE h_iocp, SOCKET l_socket)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes,
			&ikey, &over, INFINITE);
		int key = static_cast<int>(ikey);
		if (FALSE == ret) {
			if (SERVER_ID == key) {
				display_error("GQCS : ", WSAGetLastError());
				exit(-1);
			}
			else {
				display_error("GQCS : ", WSAGetLastError());
				disconnect(key);
			}
		}
		if ((key != SERVER_ID) && (0 == num_bytes)) {
			disconnect(key);
			continue;
		}
		EX_OVER* ex_over = reinterpret_cast<EX_OVER*>(over);
		switch (ex_over->m_op) {
		case OP_RECV: {
			unsigned char* packet_ptr = ex_over->m_packetbuf;
			int num_data = num_bytes + object[key].m_prev_size;
			int packet_size = packet_ptr[0];
			while (num_data >= packet_size) {
				process_packet(key, packet_ptr);
				num_data -= packet_size;
				packet_ptr += packet_size;
				if (0 >= num_data) break;
				packet_size = packet_ptr[0];
			}
			object[key].m_prev_size = num_data;
			if (0 != num_data)
				memcpy(ex_over->m_packetbuf, packet_ptr, num_data);
			do_recv(key);
		}
					break;
		case OP_SEND:
			delete ex_over;
			break;
		case OP_ACCEPT: {
			int c_id = get_new_player_id(ex_over->m_csocket);
			if (-1 != c_id) {
				object[c_id].m_recv_over.m_op = OP_RECV;
				object[c_id].m_prev_size = 0;
				CreateIoCompletionPort(
					reinterpret_cast<HANDLE>(object[c_id].m_socket), h_iocp, c_id, 0);
				do_recv(c_id);
			}
			else {
				closesocket(object[c_id].m_socket);
			}
			memset(&ex_over->m_over, 0, sizeof(ex_over->m_over));
			SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			ex_over->m_csocket = c_socket;
			AcceptEx(l_socket, c_socket,
				ex_over->m_packetbuf, 0, 32, 32, NULL, &ex_over->m_over);
		}
					  break;
		case OP_RANDOM_MOVE: //여기서 플레이어가 없으면 이동하지 않는다
			do_npc_random_move(object[key]);
			add_event(key, OP_RANDOM_MOVE, 1000);
			delete ex_over;
			break;
		case OP_ATTACK: 
			delete ex_over;
			break;
		}
	}
}


void do_ai()
{
	using namespace chrono;
	for (;;) {
		auto start_t = chrono::system_clock::now();
		for (auto& npc : object) {
			if (true == is_npc(npc.id)) {
				do_npc_random_move(npc);
			}
		}
		auto end_t = chrono::system_clock::now();
		auto ai_time = end_t - start_t;
		cout << "AI Exec Time : " << duration_cast<milliseconds>(ai_time).count()
			<<"ms\n";
		if (end_t < start_t + 1s);
			this_thread::sleep_for(start_t + 1s - end_t);
	}
	
}

void do_timer()
{
	using namespace chrono;
	for (;;) {
		timer_l.lock();
		if ((false == timer_queue.empty())&&(timer_queue.top().start_time <= system_clock::now())) {
			TIMER_EVENT ev = timer_queue.top();
			timer_queue.pop();
			timer_l.unlock();
			EX_OVER* ex_over = new EX_OVER;
			ex_over->m_op = OP_RANDOM_MOVE;
			PostQueuedCompletionStatus(h_iocp, 1, ev.object, &ex_over->m_over);
		}
		else {
			timer_l.unlock();
			this_thread::sleep_for(10ms);
		}
	}
}

int main()
{
	for (int i = 0; i < MAX_USER + 1; ++i) {
		auto& pl = object[i];
		pl.id = i;
		pl.m_state = PLST_FREE;
		if (true == is_npc(i)) {
			sprintf_s(pl.m_name, "N%d", i);
			pl.m_state = PLST_INGAME;
			pl.x = rand() % WORLD_X_SIZE;
			pl.y = rand() % WORLD_Y_SIZE;
			//add_event(i, OP_RANDOM_MOVE,1000);
		}
	}
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	wcout.imbue(locale("korean"));
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, SERVER_ID, 0);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);
	EX_OVER accept_over;
	accept_over.m_op = OP_ACCEPT;
	memset(&accept_over.m_over, 0, sizeof(accept_over.m_over));
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	accept_over.m_csocket = c_socket;
	BOOL ret = AcceptEx(listenSocket, c_socket,
		accept_over.m_packetbuf, 0, 32, 32, NULL, &accept_over.m_over);
	if (FALSE == ret) {
		int err_num = WSAGetLastError();
		if (err_num != WSA_IO_PENDING)
			display_error("AcceptEx Error", err_num);
	}
	vector <thread> worker_threads;
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(worker, h_iocp, listenSocket);

	//thread ai_thread{ do_ai };
	//ai_thread.join();

	thread timer_thread{ do_timer };
	timer_thread.join();

	for (auto& th : worker_threads)
		th.join();
	closesocket(listenSocket);
	WSACleanup();
}