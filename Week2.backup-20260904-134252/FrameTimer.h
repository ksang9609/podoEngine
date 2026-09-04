#pragma once
#include <windows.h>

class UFrameTimer
{
public:
	UFrameTimer(int TargetFPS) : targetFrameTime(1000.0 / TargetFPS), elapsedTime(1000.0 / TargetFPS)
	{
		QueryPerformanceFrequency(&Frequency);
	}

	void StartFrame()
	{
		deltaTime = (float)(elapsedTime * 0.001);
		if (deltaTime > 0.1f) deltaTime = 0.1f;

		// ���� ���� �ð� ���
		QueryPerformanceCounter(&StartTime);
	}
	void EndFrame()
	{	
		do
		{
			Sleep(0);

			// ���� ���� �ð� ���
			QueryPerformanceCounter(&EndTime);

			// �� �������� �ҿ�� �ð� ��� (�и��� ������ ��ȯ)
			elapsedTime = (EndTime.QuadPart - StartTime.QuadPart) * 1000.0 / Frequency.QuadPart;

		} while (elapsedTime < targetFrameTime);
	}

	float GetDeltaTime() const { return deltaTime; }
	float GetFPS() const { return elapsedTime > 0.0 ? (float)(1000.0 / elapsedTime) : 0.f; }

private:
	double targetFrameTime; //Ÿ�� ������ �ð�
	double elapsedTime;		//�̹� �����ӿ� �ɸ� �ð�
	float deltaTime = 0.f;	//elapsedTime�� ms -> s��
	LARGE_INTEGER Frequency, StartTime, EndTime;
};