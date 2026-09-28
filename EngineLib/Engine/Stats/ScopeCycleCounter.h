// EngineLib/Engine/Stats/ScopeCycleCounter.h

#pragma once

#include "Core/Core.h"
#include "Core/Name.h"
#include "Core/Container/TMap.h"
#include "Platform/WindowsPlatformTime.h"

using FPlatformTime = FWindowsPlatformTime;

enum class EStatId
{
	Picking,
};

struct TStatId
{
	EStatId StatId;
};

struct FCycleStat
{
	uint64 CycleCount;
	uint64 LastCycles;
	uint64 TotalCycles;
};

class FScopeCycleCounter
{
public:
	FScopeCycleCounter(TStatId StatId)
		: mStartCycles(FPlatformTime::Cycles64())
		, mUsedStatId(StatId)
	{
	}

	~FScopeCycleCounter()
	{
		Finish();
	}

	void Finish() {
		const uint64 endCycles = FPlatformTime::Cycles64();
		const uint64 cycleDiff = endCycles - mStartCycles;

		mgCycleStatMap[mUsedStatId.StatId].CycleCount++;
		mgCycleStatMap[mUsedStatId.StatId].LastCycles = cycleDiff;
		mgCycleStatMap[mUsedStatId.StatId].TotalCycles += cycleDiff;
	}

	static const FCycleStat& GetCycleStat(TStatId StatId)
	{
		const FCycleStat* stat = mgCycleStatMap.Find(StatId.StatId);
		if (!stat)
		{
			mgCycleStatMap.Add(StatId.StatId, FCycleStat{ 0, 0, 0 });
			return mgCycleStatMap[StatId.StatId];
		}
		return *stat;
	}

private:
	uint64 mStartCycles;
	TStatId mUsedStatId;

	inline static TMap<EStatId, FCycleStat> mgCycleStatMap;
};
