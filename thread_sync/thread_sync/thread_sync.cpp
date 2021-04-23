#include <iostream>
#include <thread>
#include <mutex>
//버그를 보려면 버전16.8을 사용

using namespace std;

mutex m;
int g_data = 0;
bool g_ready = false;

void receiver()
{
	m.lock();
	while (false == g_ready);
	m.unlock();
	cout << "I received " << g_data << "\n";
}

void sender()
{
	g_data = 999;
	m.lock();
	g_ready = true;
	m.unlock();
}

int main()
{
	thread t1{ receiver };
	//this_thread::sleep_for(1);
	thread t2{ sender };
	t2.join();
	t1.join();
}