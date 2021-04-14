//밴치마크테스트
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
using namespace chrono;

constexpr int LOOP_COUNT = 500000000;
volatile int sum = 0;

void worker()
{
	int loop = LOOP_COUNT / 2;
	for (int i = 0; i < loop; ++i)
		sum = sum + 2;
}

int main()
{
	//나만 안배운 volatile
	
	auto start_t = high_resolution_clock::now();
	thread t1{ worker };
	thread t2{ worker };
	t1.join();
	t2.join();
	
	auto end_t = high_resolution_clock::now();
	auto exec_t = end_t - start_t;

	cout << "Exec Time = " << duration_cast<milliseconds>(exec_t).count() <<"ms"<< endl;
	cout << "Result = " << sum << endl;
}