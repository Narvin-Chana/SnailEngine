#pragma once
#include <stdint.h>

namespace Snail
{
	class Clock
	{
	public:
		Clock();
		int64_t GetTimeCount() const;
		double GetSecPerCount() const noexcept { return secondsPerCount; }
		// retourne le temps en millisecondes entre deux count.
		double GetTimeBetweenCounts(int64_t start, int64_t stop) const;
	private:
		double secondsPerCount;
	};
} // namespace PM3