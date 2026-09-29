// EngineLib/Rendering/GpuTimer.h

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class FGpuTimer
{
public:
	HRESULT Initialize(ID3D11Device* device);
	void Shutdown();

	// 이전 프레임들의 결과 확인. 기다리지 않음.
	HRESULT Poll(ID3D11DeviceContext* context);

	// 프레임당 각각 한 번 호출.
	void Begin(ID3D11DeviceContext* context);
	void End(ID3D11DeviceContext* context);

	bool HasResult() const { return mHasResult; }
	double GetMilliseconds() const { return mLastMilliseconds; }

private:
	static constexpr UINT SlotCount = 4;

	struct FSlot
	{
		Microsoft::WRL::ComPtr<ID3D11Query> Disjoint;
		Microsoft::WRL::ComPtr<ID3D11Query> Start;
		Microsoft::WRL::ComPtr<ID3D11Query> Finish;
	};

	FSlot mSlots[SlotCount];

	UINT mReadIndex = 0;
	UINT mWriteIndex = 0;
	UINT mPendingCount = 0;

	bool mReady = false;
	bool mRecording = false;
	bool mHasResult = false;

	double mLastMilliseconds = 0.0;
};
