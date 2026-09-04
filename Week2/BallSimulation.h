#pragma once

#include "Ball.h"

class FBallSimulation
{
public:
    FBallSimulation(int maxBalls);
    ~FBallSimulation();

    void SetBallCount(int count);
    void Simulate(float deltaTime);

    int GetBallCount() const { return UBall::TotalNumBalls; }
    int GetMaxBallCount() const { return MaxBall; }
    UBall* GetBall(int i) const { return static_cast<UBall*>(PrimitiveList[i]); }

    bool bGravity = true;

private:
    void ApplyGravity(float dt, UBall& ball);
    void ClampWalls(UBall& ball);
    void ResolveBallCollisions();
    void SpawnBalls(int count);
    void DeleteBalls(int count);

    int MaxBall;
    UPrimitive** PrimitiveList;

    const float kSleepVelocity = 0.05f;
	const float kRestitution = 0.8f;

    const float leftBorder = -1.0f;
    const float rightBorder = 1.0f;
    const float topBorder = -1.0f;
    const float bottomBorder = 1.0f;

};
