#include <iostream>
#include <thread>

//버그를 보려면 버전16.8을 사용

using namespace std;

int g_data = 0;
volatile bool g_ready = false;

void receiver()
{
	while (false == g_ready);
	cout << "I received " << g_data << "\n";
}

void sender()
{
	g_data = 999;
	g_ready = true;
}

int main()
{
	thread t1{ receiver };
	//this_thread::sleep_for(1);
	thread t2{ sender };
	t2.join();
	t1.join();
}