#include<iostream>
#include<thread>

void do_thread() 
{
	std::cout << "Iam a anoter thread: "<<std::this_thread::get_id()<<"\n";
}

int main()
{

	std::thread t1{ do_thread };
	std::thread t2{ do_thread };
	std::thread t3{ do_thread };
	std::thread t4{ do_thread };
	std::thread t5{ do_thread };

	t1.join();
}
