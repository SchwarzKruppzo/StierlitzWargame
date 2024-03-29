#pragma once

#include <thread>

bool bStopThreads = false;

class pTimer
{
public:
	template <typename T>
	void setLoop(T func, uint64_t ms)
	{
		std::thread([=]()
			{
				while (true)
				{
					if (bStopThreads) {
						break;
					}

					func();
					std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				}
			}).detach();
	}

	template <typename T>
	void setWait(T func, uint64_t ms)
	{
		std::thread([=]()
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				func();
			}).detach();
	}
};
