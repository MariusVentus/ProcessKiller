#include "TimeKeeper.h"

using namespace std::chrono;

TimeKeeper::TimeKeeper()
{
	last = steady_clock::now();
}

float TimeKeeper::Mark()
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> epochTime = last - old;
	return epochTime.count();
}

bool TimeKeeper::IntervalCheck(float interval)
{
	const duration<float> dur = steady_clock::now() - last;
	if (dur.count() >= interval) {
		Mark();
		return true;
	}
	else {
		return false;
	}
}


