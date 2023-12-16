#include "stdafx.h"

#include "Clock.h"

namespace Snail
{
	Clock::Clock()
	{
		LARGE_INTEGER counterFrequency;
		QueryPerformanceFrequency(&counterFrequency);
		secondsPerCount = 1.0 / static_cast<double>(counterFrequency.QuadPart);
	}

	int64_t Clock::GetTimeCount() const
	{
		LARGE_INTEGER countNumber;
		QueryPerformanceCounter(&countNumber);
		return countNumber.QuadPart;
	}

	double Clock::GetTimeBetweenCounts(int64_t start, int64_t stop) const
	{
		return static_cast<double>(stop - start) * secondsPerCount;
	}
} // namespace Snail