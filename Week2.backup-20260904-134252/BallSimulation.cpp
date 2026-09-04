#include "BallSimulation.h"
#include <cmath>

FBallSimulation::FBallSimulation(int maxBalls) : MaxBall(maxBalls)
{
	PrimitiveList = new UPrimitive*[MaxBall];

	//ù �� ����
	PrimitiveList[0] = new UBall(FVector(0.f),
		FVector((float)(rand() % 100 - 50) * 0.06f, (float)(rand() % 100 - 50) * 0.06f, 0.f)
		, 0.1f);
}

FBallSimulation::~FBallSimulation()
{
	// �Ҹ��ϴ� �ڵ带 ���⿡ �߰��մϴ�.
	int Count = UPrimitive::TotalNumBalls;
	for (int i = 0; i < Count; i++)
		delete PrimitiveList[i];
	delete[] PrimitiveList;
}

void FBallSimulation::SetBallCount(int count)
{
	if (count != UBall::TotalNumBalls)
	{
		if (count > UBall::TotalNumBalls)
		{
			SpawnBalls(count - UBall::TotalNumBalls);
		}
		else if (count < UBall::TotalNumBalls)
		{
			DeleteBalls(UBall::TotalNumBalls - count);
		}
	}
}

void FBallSimulation::Simulate(float deltaTime)
{
	for (int i = 0; i < UBall::TotalNumBalls; i++)
	{
		UBall* ball = GetBall(i);
		if (bGravity) ApplyGravity(deltaTime, *ball);
		ball->Move(deltaTime);
		ClampWalls(*ball);
	}

	ResolveBallCollisions();
}

void FBallSimulation::ApplyGravity(float dt, UBall& ball)
{
	float gravityAccel = 4.f * dt;
	ball.Velocity.y -= gravityAccel;
}

void FBallSimulation::ClampWalls(UBall& ball)
{
	// ���� �浹 ���θ� üũ�ϰ� �浹�� �ӵ��� ������ ���� ������ �ٲ�
	if (ball.Location.x < leftBorder + ball.Radius)
	{
		ball.Location.x = leftBorder + ball.Radius;
		if (ball.Velocity.x < 0.f)
		{
			if (-ball.Velocity.x >= kSleepVelocity)
				ball.Velocity.x *= -kRestitution;
			else
				ball.Velocity.x = 0.f;
		}
	}
	if (ball.Location.x > rightBorder - ball.Radius)
	{
		ball.Location.x = rightBorder - ball.Radius;
		if (ball.Velocity.x > 0.f)
		{
			if (ball.Velocity.x >= kSleepVelocity)
				ball.Velocity.x *= -kRestitution;
			else
				ball.Velocity.x = 0.f;
		}
	}
	if (ball.Location.y < topBorder + ball.Radius)
	{
		ball.Location.y = topBorder + ball.Radius;
		if (ball.Velocity.y < 0.f)
		{
			if (-ball.Velocity.y >= kSleepVelocity)
				ball.Velocity.y *= -kRestitution;
			else
				ball.Velocity.y = 0.f;
		}
	}
	if (ball.Location.y > bottomBorder - ball.Radius)
	{
		ball.Location.y = bottomBorder - ball.Radius;
		if (ball.Velocity.y > 0.f)
		{
			if (ball.Velocity.y >= kSleepVelocity)
				ball.Velocity.y *= -kRestitution;
			else
				ball.Velocity.y = 0.f;
		}
	}
}

void FBallSimulation::ResolveBallCollisions()
{
	for (int i = 0; i < UBall::TotalNumBalls; i++)
	{
		UBall* A = static_cast<UBall*>(PrimitiveList[i]);
		for (int j = i + 1; j < UBall::TotalNumBalls; ++j)
		{
			//�΋H���ٸ�
			if (A->Compact(PrimitiveList[j]))
			{
				UBall* B = static_cast<UBall*>(PrimitiveList[j]);

				//���� ���ϰ� ����ȭ
				FVector normal = B->Location - A->Location;
				float dist = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
				if (dist < 1e-6f)
				{
					// ���� �������� �о
					normal = FVector(1.f, 0.f, 0.f);
					dist = 1e-6f;
				}
				normal.x /= dist;
				normal.y /= dist;
				normal.z /= dist;

				//�࿡ �� ���� �ӵ��� ����
				float v1n = FVector::dot(A->Velocity, normal);
				float v2n = FVector::dot(B->Velocity, normal);

				//A�� B�� ��������� ������ �浹ó��, �־����� �ִٸ� �浹ó���� ���Ѵ�.
				if (v2n - v1n < 0.f)
				{
					float m1 = A->Mass, m2 = B->Mass;
					float v1nAfter = ((m1 - m2) * v1n + m2 * (1 + kRestitution) * v2n) / (m1 + m2);
					float v2nAfter = ((m2 - m1) * v2n + m1 * (1 + kRestitution) * v1n) / (m1 + m2);
					A->Velocity += (v1nAfter - v1n) * normal;
					B->Velocity += (v2nAfter - v2n) * normal;
				}

				//���� ��ģ ���·� ���θ� ���� �����ӿ��� �ٽ� ź�� �浹�� �Ͼ�Ƿ�
				//��ģ ���̸�ŭ �и�
				float overlap = (A->Radius + B->Radius) - dist;
				if (overlap > 0.f)
				{
					FVector correction(normal.x * overlap * 0.5f, normal.y * overlap * 0.5f, normal.z * overlap * 0.5f);
					A->Location -= correction;
					B->Location += correction;
				}
			}
		}
	}
}

void FBallSimulation::SpawnBalls(int count)
{
	for (int i = 0; i < count; ++i)
	{
		//��ġ, �ӵ� �Ѵ� ��������
		UBall* newBall = new UBall(FVector(((float)(rand() % 200 - 100)) * 0.005f, ((float)(rand() % 200 - 100)) * 0.005f, 0.0f),
			FVector((float)(rand() % 100 - 50) * 0.06f, (float)(rand() % 100 - 50) * 0.06f, 0.f),
			(float)(rand() % 15 + 1) * 0.01f);
		PrimitiveList[UPrimitive::TotalNumBalls - 1] = newBall;
	}
}

void FBallSimulation::DeleteBalls(int count)
{
	for (int i = 0; i < count; ++i)
	{
		int randi = rand() % UBall::TotalNumBalls;
		UPrimitive* temp = PrimitiveList[randi];
		PrimitiveList[randi] = PrimitiveList[UBall::TotalNumBalls - 1];

		PrimitiveList[UPrimitive::TotalNumBalls - 1] = nullptr;
		delete temp;
	}
}
