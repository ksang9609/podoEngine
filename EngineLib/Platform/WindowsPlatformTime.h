// EngineLib/Platform/WindowsPlatformTime.h

#pragma once

#include <Windows.h>

class FWindowsPlatformTime
{
public:
	static void InitTiming() {
		if (!mbInitialized)
		{
			mbInitialized = true;

			double Frequency = (double)GetFrequency();
			if (Frequency <= 0.0)
			{
				Frequency = 1.0;
			}

			mgSecondsPerCycle = 1.0 / Frequency;
		}
	}

	static float GetSecondsPerCycle() {
		if (!mbInitialized)
		{
			InitTiming();
		}
		return (float)mgSecondsPerCycle;
	}

	static uint64 GetFrequency() {
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		return Frequency.QuadPart;
	}

	static double ToMilliseconds(uint64 CycleDiff) {
		double Ms = static_cast<double>(CycleDiff)
			* GetSecondsPerCycle()
			* 1000.0;

		return Ms;
	}

	static uint64 Cycles64() {
		LARGE_INTEGER CycleCount;
		QueryPerformanceCounter(&CycleCount);
		return (uint64)CycleCount.QuadPart;
	}

private:
	inline static double mgSecondsPerCycle; // 0
	inline static bool mbInitialized; // false
};
