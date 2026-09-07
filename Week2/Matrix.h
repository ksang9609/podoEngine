#pragma once
#include "Vector.h"
#include "MathUtility.h"
#include "Rotator.h"
#include "enum.h"

struct FMatrix { 
	float M[4][4];

	static const FMatrix Identity;
	static const FMatrix Zero;

	// 언리얼 좌표계(X 전방 / Y 우측 / Z 상방)를
	// DirectX NDC(X 우측 / Y 위 / Z 화면 안쪽)로 바꾸는 축 교환 행렬.
	static const FMatrix UEToDX;
	

	static FMatrix makeIdentity() // 단위행렬 만드는 함수
	{
		FMatrix R = {};
		R.M[0][0] = R.M[1][1] = R.M[2][2] = R.M[3][3] = 1.0f;
		return R;
	}

	FMatrix operator* (const FMatrix& Other) const  // 4 x 4 행렬곱
	{
		FMatrix result = {};

		for (int row = 0; row < 4;++row) {
			for (int col = 0;col < 4;++col) {
				for (int k = 0;k < 4;++k) {
					result.M[row][col] += M[row][k] * Other.M[k][col];
				}
			}
		}
		return result;
	}

	FMatrix operator*(float Scalar) const  // M x 상수(Sclalar) ,Scaling 아님
	{ 
		FMatrix result;
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] * Scalar;
			}
		}

		return result;
	}


	FMatrix operator+ (const FMatrix& Other) const   // M + M , 행렬끼리의 합
	{ 
		FMatrix result = {};

		for (int row = 0; row < 4;++row) {
			for (int col = 0;col < 4;++col) {
				result.M[row][col] = M[row][col] + Other.M[row][col];
			}
		}
		return result;
	}



	FMatrix operator- (const FMatrix& Other) const  // M - M , 행렬끼리의 차
	{ 
		FMatrix result = {};

		for (int row = 0; row < 4;++row) {
			for (int col = 0;col < 4;++col) {
				result.M[row][col] = M[row][col] - Other.M[row][col];
			}
		}
		return result;
	}


	FMatrix operator+(float f) const // M + f : 각원소 덧셈(f)
	{ 
		FMatrix result = {};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] + f;
			}
		}
		return result;
	}

	FMatrix operator-(float f) const // M - f : 각원소 뺄셈(f)
	{ 
		FMatrix result={};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] - f;
			}
		}
		return result;
	}



	FMatrix Transpose() const // 전치행렬
	{ 
		FMatrix result = {};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[col][row];
			}
		}
		return result;
	}

	static FMatrix Scale(float n) // scaling, 상수배
	  {
		FMatrix result = Identity;
		result.M[0][0] = n;
		result.M[1][1] = n;
		result.M[2][2] = n;

		return result;
	}

	static FMatrix Scale(const FVector v) // scaling, xyz배율만큼
	{
		FMatrix result = Identity;
		result.M[0][0] = v.x;
		result.M[1][1] = v.y;
		result.M[2][2] = v.z;

		return result;
	}

	static FMatrix RotateX(float degree) // Roll : X축 회전, yz평면
	{
		FMatrix result = Identity;
		float s, c;
		FMath::sincos<float>(s, c, degree * PI / 180);

		result.M[1][1] = c;
		result.M[1][2] = -s;
		result.M[2][1] = s;
		result.M[2][2] = c;

		return result;
	}

	static FMatrix RotateY(float degree) // Pitch : Y축 회전, zx평면
	{
		FMatrix result = Identity;
		float s, c;
		FMath::sincos<float>(s, c, degree * PI / 180);

		result.M[0][0] = c;
		result.M[0][2] = s;
		result.M[2][0] = -s;
		result.M[2][2] = c;

		return result;
	}

	static FMatrix RotateZ(float degree) // Yaw : Z축 회전, xy평면
	{
		FMatrix result = Identity;
		float s, c;
		FMath::sincos<float>(s, c, degree * PI / 180);

		result.M[0][0] = c;
		result.M[0][1] = s;
		result.M[1][0] = -s;
		result.M[1][1] = c;

		return result;
	}

	static FMatrix Rotate(const FRotator r)
	{
		//Pitch, Yaw, Roll의 각각 cossin 구하기
		FMatrix Matrix = FMatrix::Identity;
		float cosP, cosY, cosR;
		float sinP, sinY, sinR;

		//도 -> 라디안 변환 후 sincos 호출
		FMath::sincos<float>(sinP, cosP, r.Pitch * PI / 180);
		FMath::sincos<float>(sinY, cosY, r.Yaw * PI / 180);
		FMath::sincos<float>(sinR, cosR, r.Roll * PI / 180);

		Matrix.M[0][0] = cosP * cosY;
		Matrix.M[0][1] = cosP * sinY;
		Matrix.M[0][2] = sinP;
		Matrix.M[1][0] = sinR * sinP * cosY - cosR * sinY;
		Matrix.M[1][1] = sinR * sinP * sinY + cosR * cosY;
		Matrix.M[1][2] = -sinR * cosP;
		Matrix.M[2][0] = -(cosR * sinP * cosY + sinR * sinY);
		Matrix.M[2][1] = sinR * cosY - cosR * sinP * sinY;
		Matrix.M[2][2] = cosR * cosP;

		return Matrix;
	}

	static FMatrix Translation(const FVector v) // translate
	{
		FMatrix result = Identity;
		result.M[3][0] = v.x;
		result.M[3][1] = v.y;
		result.M[3][2] = v.z;

		return result;
	}

	[[nodiscard]] FVector GetUnitAxis(EAxis Axis) const
	{
		const int i = static_cast<int>(Axis);
		return FVector(M[i][0], M[i][1], M[i][2]);
	}

	// 위치 변환 (w = 1, 이동 포함).  행벡터 규약 v x M
	[[nodiscard]] FVector TransformPosition(const FVector& V) const
	{
		return FVector(
			V.x * M[0][0] + V.y * M[1][0] + V.z * M[2][0] + M[3][0],
			V.x * M[0][1] + V.y * M[1][1] + V.z * M[2][1] + M[3][1],
			V.x * M[0][2] + V.y * M[1][2] + V.z * M[2][2] + M[3][2]);
	}

	// 방향 변환 (w = 0, 이동 제외)
	[[nodiscard]] FVector TransformVector(const FVector& V) const
	{
		return FVector(
			V.x * M[0][0] + V.y * M[1][0] + V.z * M[2][0],
			V.x * M[0][1] + V.y * M[1][1] + V.z * M[2][1],
			V.x * M[0][2] + V.y * M[1][2] + V.z * M[2][2]);
	}

	// 아핀 행렬(마지막 열이 0,0,0,1)의 역행렬.
	// MakeMatrix() 결과가 항상 이 형태라 일반 4x4 역행렬이 필요 없다.
	//   M = | A 0 |        M^-1 = | A^-1     0 |
	//       | t 1 |               | -t*A^-1  1 |
	// Transpose() 와 달리 비균등 스케일에도 동작한다.
	[[nodiscard]] FMatrix Inverse() const
	{
		const float C00 =  (M[1][1] * M[2][2] - M[1][2] * M[2][1]);
		const float C01 = -(M[1][0] * M[2][2] - M[1][2] * M[2][0]);
		const float C02 =  (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

		const float Det = M[0][0] * C00 + M[0][1] * C01 + M[0][2] * C02;
		if (FMath::Abs(Det) < SMALL_NUMBER)
		{
			return FMatrix::Identity;   // 스케일 0 등 역행렬이 없는 경우
		}

		const float C10 = -(M[0][1] * M[2][2] - M[0][2] * M[2][1]);
		const float C11 =  (M[0][0] * M[2][2] - M[0][2] * M[2][0]);
		const float C12 = -(M[0][0] * M[2][1] - M[0][1] * M[2][0]);
		const float C20 =  (M[0][1] * M[1][2] - M[0][2] * M[1][1]);
		const float C21 = -(M[0][0] * M[1][2] - M[0][2] * M[1][0]);
		const float C22 =  (M[0][0] * M[1][1] - M[0][1] * M[1][0]);

		const float Inv = 1.0f / Det;

		FMatrix R = FMatrix::Identity;
		// 수반행렬 = 여인수 행렬의 전치
		R.M[0][0] = C00 * Inv;  R.M[0][1] = C10 * Inv;  R.M[0][2] = C20 * Inv;
		R.M[1][0] = C01 * Inv;  R.M[1][1] = C11 * Inv;  R.M[1][2] = C21 * Inv;
		R.M[2][0] = C02 * Inv;  R.M[2][1] = C12 * Inv;  R.M[2][2] = C22 * Inv;

		// 이동 성분 : -t * A^-1
		R.M[3][0] = -(M[3][0] * R.M[0][0] + M[3][1] * R.M[1][0] + M[3][2] * R.M[2][0]);
		R.M[3][1] = -(M[3][0] * R.M[0][1] + M[3][1] * R.M[1][1] + M[3][2] * R.M[2][1]);
		R.M[3][2] = -(M[3][0] * R.M[0][2] + M[3][1] * R.M[1][2] + M[3][2] * R.M[2][2]);

		return R;
	}


	// end Struct Matrix
};

inline const FMatrix FMatrix::Identity = { {
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
} };

inline const FMatrix FMatrix::Zero = { {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
} };


//축 교환 행렬
inline const FMatrix FMatrix::UEToDX = { {
	{0, 0, 1, 0},   // UE X(전방) -> DX Z(화면 안쪽)
	{1, 0, 0, 0},   // UE Y(우측) -> DX X(우측)
	{0, 1, 0, 0},   // UE Z(상방) -> DX Y(위)
	{0, 0, 0, 1}
} };
