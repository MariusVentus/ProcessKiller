#pragma once
#include <chrono>

class TimeKeeper {
public:
	TimeKeeper();
	float Mark();
	bool IntervalCheck(float interval);

private:
	std::chrono::steady_clock::time_point last;
};