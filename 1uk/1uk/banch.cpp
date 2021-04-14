//밴치마크테스트
#include <iostream>
#include <chrono>
using namespace std;
using namespace chrono;

constexpr int LOOP_COUNT = 500000000;
int main()
{
	//나만 안배운 volatile
	volatile int sum = 0;
	auto start_t = high_resolution_clock::now();
	for (int i = 0; i < LOOP_COUNT; ++i)
		sum = sum + 2;
	auto end_t = high_resolution_clock::now();
	auto exec_t = end_t - start_t;

	cout << "Exec Time = " << duration_cast<milliseconds>(exec_t).count() <<"ms"<< endl;
	cout << "Result = " << sum << endl;
}