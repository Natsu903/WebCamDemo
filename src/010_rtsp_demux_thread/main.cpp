#include <iostream>
#include <thread>
#include <chrono>
#include "tools.h"

class TestThread :public Demuxthread
{
public:
	void Main()
	{
		LOGDEBUG("TestThread Main Begin");
		while (!is_exit_)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		LOGDEBUG("TestThread Main end");
	}
};

int main(int argc, char* argv[])
{
	TestThread tt;
	tt.Start();
	std::this_thread::sleep_for(std::chrono::seconds(3));
	tt.Stop();
	getchar();
	return 0;
}
