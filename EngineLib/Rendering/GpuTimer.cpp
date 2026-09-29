// EngineLib/Rendering/GpuTimer.cpp

#include "GpuTimer.h"

HRESULT FGpuTimer::Initialize(ID3D11Device* device)
{
	Shutdown();

	if (!device)
		return E_INVALIDARG;

	for (auto& slot : mSlots)
	{
		D3D11_QUERY_DESC desc = {};
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

		HRESULT hr = device->CreateQuery(
			&desc, slot.Disjoint.GetAddressOf());

		if (FAILED(hr))
		{
			Shutdown();
			return hr;
		}

		desc.Query = D3D11_QUERY_TIMESTAMP;

		hr = device->CreateQuery(
			&desc, slot.Start.GetAddressOf());

		if (FAILED(hr))
		{
			Shutdown();
			return hr;
		}

		hr = device->CreateQuery(
			&desc, slot.Finish.GetAddressOf());

		if (FAILED(hr))
		{
			Shutdown();
			return hr;
		}
	}

	mReady = true;
	return S_OK;
}

void FGpuTimer::Shutdown()
{
	// 정상 사용에서는 Begin/End가 끝난 뒤 호출.
	for (auto& slot : mSlots)
	{
		slot.Disjoint.Reset();
		slot.Start.Reset();
		slot.Finish.Reset();
	}

	mReadIndex = 0;
	mWriteIndex = 0;
	mPendingCount = 0;

	mReady = false;
	mRecording = false;
	mHasResult = false;
	mLastMilliseconds = 0.0;
}

void FGpuTimer::Begin(ID3D11DeviceContext* context)
{
	if (!mReady || mRecording)
		return;

	// 모든 세트가 처리 중이면 이번 프레임 측정만 생략.
	// 렌더링 자체는 계속 진행.
	if (mPendingCount == SlotCount)
		return;

	auto& slot = mSlots[mWriteIndex];

	context->Begin(slot.Disjoint.Get());
	context->End(slot.Start.Get());

	mRecording = true;
}

void FGpuTimer::End(ID3D11DeviceContext* context)
{
	// Begin에서 측정을 생략했거나 시작하지 않았다면 아무 일도 안 함.
	if (!mRecording)
		return;

	auto& slot = mSlots[mWriteIndex];

	context->End(slot.Finish.Get());
	context->End(slot.Disjoint.Get());

	mRecording = false;
	++mPendingCount;
	mWriteIndex = (mWriteIndex + 1) % SlotCount;
}

HRESULT FGpuTimer::Poll(ID3D11DeviceContext* context)
{
	if (!mReady)
		return S_FALSE;

	constexpr UINT flags = D3D11_ASYNC_GETDATA_DONOTFLUSH;

	// 준비된 세트를 오래된 순서대로 회수.
	while (mPendingCount > 0)
	{
		auto& slot = mSlots[mReadIndex];

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timing = {};
		UINT64 start = 0;
		UINT64 finish = 0;

		HRESULT hr = context->GetData(
			slot.Disjoint.Get(), &timing, sizeof(timing), flags);

		if (hr == S_FALSE)
			return S_FALSE; // 다음 프레임에 다시 확인

		if (FAILED(hr))
			return hr;

		// 유효한 측정일 때만 Timestamp를 읽고 반영.
		if (!timing.Disjoint && timing.Frequency != 0)
		{
			hr = context->GetData(
				slot.Start.Get(), &start, sizeof(start), flags);

			if (hr == S_FALSE)
				return S_FALSE;

			if (FAILED(hr))
				return hr;

			hr = context->GetData(
				slot.Finish.Get(), &finish, sizeof(finish), flags);

			if (hr == S_FALSE)
				return S_FALSE;

			if (FAILED(hr))
				return hr;

			if (finish >= start)
			{
				mLastMilliseconds =
					static_cast<double>(finish - start)
					/ static_cast<double>(timing.Frequency)
					* 1000.0;

				mHasResult = true;
			}
		}

		// 정상 결과는 반영하고, Disjoint 결과는 버림.
		// 둘 다 이 세트는 재사용 가능.
		--mPendingCount;
		mReadIndex = (mReadIndex + 1) % SlotCount;
	}

	return S_OK;
}
