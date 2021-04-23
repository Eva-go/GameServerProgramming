#include <iostream>
#include <thread>
//버그를 보려면 버전16.8을 사용
using namespace std;

volatile int sum = 0;
volatile int v = 0;
volatile bool flags[2] = { false,false };

void p_lock(int t_id)
{
	int other = 1 - t_id;
	flags[t_id] = true;
	v = t_id;
	atomic_thread_fence(memory_order_seq_cst);
	while ((true == flags[other]) && v == t_id);
}

void p_unlock(int t_id)
{
	flags[t_id] = false;
}

void worker(int t_id)
{
	for (int i = 0; i < 25000000; ++i) {
		p_lock(t_id);
		sum = sum + 2;
		p_unlock(t_id);
	}
}

int main()
{
	thread t1{ worker,0 };
	//this_thread::sleep_for(1);
	thread t2{ worker,1 };
	t2.join();
	t1.join();

	cout << "SUM : " << sum;
}