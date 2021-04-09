#include<iostream>
#include<thread>
#include<vector>
void do_thread(int th_id) 
{
	std::cout << "Iam a anoter thread: "<<th_id<<"\n";
}

int main()
{
	std::vector<std::thread> my_threads;
	for (int i = 0; i < 8; ++i) {
		my_threads.emplace_back(do_thread, i);
	}
	for (auto& th : my_threads)
		th.join();
}
