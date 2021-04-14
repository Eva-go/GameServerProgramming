//밴치마크테스트
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;
using namespace chrono;

#define MAX_THREADS 16

constexpr int LOOP_COUNT = 500000000;
volatile int sum = 0;

void worker(int num_threads)
{
	int loop = LOOP_COUNT / num_threads;
	for (int i = 0; i < loop; ++i)
		sum = sum + 2;
}

int main()
{
	vector <thread> workers;
	//나만 안배운 volatile
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		sum = 0;
		workers.clear();
		auto start_t = high_resolution_clock::now();
		
		for (int i = 0; i < num_threads; ++i)
			workers.emplace_back(worker,num_threads);
		for (auto& th : workers)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		cout << num_threads << "threads, ";
		cout << "Exec Time = " << duration_cast<milliseconds>(exec_t).count() << "ms";
		cout << "Result = " << sum << endl;
	}
	


}