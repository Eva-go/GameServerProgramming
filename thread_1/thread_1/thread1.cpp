#include<iostream>
#include<thread>

void do_thread() 
{
	std::cout << "Iam a anoter thread\n";
}

int main()
{
	std::thread t1{do_thread};
	std::cout << "hello world\n";
	t1.join();
}
