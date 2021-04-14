#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
const auto MAX_THREADS = 16;
using namespace std;
using namespace std::chrono;

struct NUM {
	volatile alignas(64) int num;
};

NUM sum[MAX_THREADS];
mutex g_lock;
void thread_func(int num_threads,int th_id)
{
	
	for (auto i = 0; i < 50000000 / num_threads; ++i) {
		sum[th_id].num = sum[th_id].num + 2;
	}
	
}

int main()
{
	vector<thread> threads;
	for (auto i = 1; i <= MAX_THREADS; i *= 2)
	{
		for (auto& s : sum) s.num = 0;
		threads.clear();
		auto start = high_resolution_clock::now();

		for (auto j = 0; j < i; ++j)
			threads.emplace_back(thread{ thread_func, i ,j});

		for (auto& tmp : threads)
			tmp.join();

		int t_sum = 0;
		for (auto& s : sum)
			t_sum = t_sum + s.num;
		auto duration = high_resolution_clock::now() - start;
		cout << i << " Threads" << "  Sum = " << t_sum;
		cout << " Duration = " << duration_cast<milliseconds>(duration).count() << " milliseconds\n";
	}
}
