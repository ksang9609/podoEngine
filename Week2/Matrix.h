#pragma once
#include "Vector.h"
struct FMatrix {

	float M[4][4];
	static const FMatrix Identity;

	FMatrix Inverse() const; // 4 x 4 역행렬
			


	static FMatrix makeIdentity() // 단위행렬 만드는 함수
	{
		FMatrix R = {};
		R.M[0][0] = R.M[1][1] = R.M[2][2] = R.M[3][3] = 1.0f;
		return R;
	}

	inline FMatrix operator* (const FMatrix& Other) const  // 4 x 4 행렬곱
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

	inline FMatrix operator*(float Scalar) const  // M x 상수(Sclalar) ,Scaling 아님
	{ 
		FMatrix result;
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] * Scalar;
			}
		}

		return result;
	}


	inline FMatrix operator+ (const FMatrix& Other) const   // M + M , 행렬끼리의 합
	{ 
		FMatrix result = {};

		for (int row = 0; row < 4;++row) {
			for (int col = 0;col < 4;++col) {
				result.M[row][col] = M[row][col] + Other.M[row][col];
			}
		}
		return result;
	}



	inline FMatrix operator- (const FMatrix& Other) const  // M - M , 행렬끼리의 차
	{ 
		FMatrix result = {};

		for (int row = 0; row < 4;++row) {
			for (int col = 0;col < 4;++col) {
				result.M[row][col] = M[row][col] - Other.M[row][col];
			}
		}
		return result;
	}


	inline FMatrix operator+(float f) const // M + f : 각원소 덧셈(f)
	{ 
		FMatrix result = {};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] + f;
			}
		}
		return result;
	}

	inline FMatrix operator-(float f) const // M - f : 각원소 뺄셈(f)
	{ 
		FMatrix result={};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[row][col] - f;
			}
		}
		return result;
	}



	static FMatrix Transpose()  // 전치행렬
	{ 
		FMatrix result = {};
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				result.M[row][col] = M[col][row];
			}
		}
		return result;
	};

	static FMatrix Scale(float n) // scaling, 상수배
	{
		FMatrix result = Identity;
		result[0][0] = n;
		result[1][1] = n;
		result[2][2] = n;

		return result;
	}

	static FMatrix Scale(const FVector v) // scaling, xyz배율만큼
	{
		FMatrix result = Identity;
		result[0][0] = v.x;
		result[1][1] = v.y;
		result[2][2] = v.z;

		return result;
	}

	static FMatrix Translation(const float n) // translate
	{
		FMatrix result = Identity;
		result[3][0] = n;
		result[3][1] = n;
		result[3][2] = n;

		return result;
	}
	static FMatrix Translation(const FVector v) // translate
	{
		FMatrix result = Identity;
		result[3][0] = v.x;
		result[3][1] = v.y;
		result[3][2] = v.z;

		return result;
	}
	// end Struct Matrix
};

inline const FMatrix FMatrix::Identity = { {
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
} };

