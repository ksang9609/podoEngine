#pragma once

inline FMatrix FMatrix::makeIdentity()
{
	FMatrix R = {};
	R.M[0][0] = R.M[1][1] = R.M[2][2] = R.M[3][3] = 1.0f;
	return R;
}

inline FMatrix FMatrix::operator* (const FMatrix& Other) const
{
	FMatrix result;

	__m128 B0 = _mm_load_ps(&Other.M[0][0]);
	__m128 B1 = _mm_load_ps(&Other.M[1][0]);
	__m128 B2 = _mm_load_ps(&Other.M[2][0]);
	__m128 B3 = _mm_load_ps(&Other.M[3][0]);

	for (int i = 0; i < 4; i++)
	{
		// A의 i번째 행 가져옴
		__m128 A = _mm_load_ps(&M[i][0]);
		// i번째 행의 0번째 원소들을 복사해서 128비트 레지스터 4칸에 채워 넣는다
		__m128 A0 = _mm_shuffle_ps(A, A, _MM_SHUFFLE(0, 0, 0, 0));
		// 행벡터의 0번째 원소들을 복사해서 만든 4칸 레지스터에 곱하려는 행렬의 0번째 행을 곱한다
		__m128 Res = _mm_mul_ps(A0, B0);
		__m128 A1 = _mm_shuffle_ps(A, A, _MM_SHUFFLE(1, 1, 1, 1));
		Res = _mm_add_ps(Res, _mm_mul_ps(A1, B1));
		__m128 A2 = _mm_shuffle_ps(A, A, _MM_SHUFFLE(2, 2, 2, 2));
		Res = _mm_add_ps(Res, _mm_mul_ps(A2, B2));
		__m128 A3 = _mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 3, 3, 3));
		Res = _mm_add_ps(Res, _mm_mul_ps(A3, B3));

		_mm_store_ps(&result.M[i][0], Res);
	}
	return result;
}

inline FMatrix FMatrix::operator*(float Scalar) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(Scalar);

	for (int i = 0; i < 4; i++)
	{
		__m128 A = _mm_load_ps(&M[i][0]);
		__m128 Res = _mm_mul_ps(A, B);

		_mm_store_ps(&result.M[i][0], Res);
	}

	return result;
}

inline FMatrix FMatrix::operator+ (const FMatrix& Other) const
{
	FMatrix result;

	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_load_ps(&M[i][0]);
		__m128 iB = _mm_load_ps(&Other.M[i][0]);
		_mm_store_ps(&result.M[i][0], _mm_add_ps(iA, iB));
	}
	return result;
}

inline FMatrix FMatrix::operator- (const FMatrix& Other) const
{
	FMatrix result;

	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_load_ps(&M[i][0]);
		__m128 iB = _mm_load_ps(&Other.M[i][0]);
		_mm_store_ps(&result.M[i][0], _mm_sub_ps(iA, iB));
	}
	return result;
}

inline FMatrix FMatrix::operator+(float f) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(f);
	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_load_ps(&M[i][0]);
		_mm_store_ps(&result.M[i][0], _mm_add_ps(iA, B));
	}
	return result;
}

inline FMatrix FMatrix::operator-(float f) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(f);
	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_load_ps(&M[i][0]);
		_mm_store_ps(&result.M[i][0], _mm_sub_ps(iA, B));
	}
	return result;
}

inline bool FMatrix::operator==(const FMatrix& m) const
{
	for (int i = 0; i < 4; i++)
	{
		__m128 A = _mm_load_ps(&M[i][0]);
		__m128 B = _mm_load_ps(&m.M[i][0]);

		// 두 행의 4개 원소가 같으면 0xFFFFFFFF, 다르면 0x0
		__m128 Cmp = _mm_cmpeq_ps(A, B);
		// movemask로 비트 압축
		if (_mm_movemask_ps(Cmp) != 0xF)
		{
			return false;
		}
	}
	return true;
}

inline bool FMatrix::operator!=(const FMatrix& m) const
{
	return !(*this == m);
}

// 실무에서는 Equals가 더 많이 쓰임
inline bool FMatrix::Equals(const FMatrix& m, float Tolerance) const
{
	__m128 Tol = _mm_set1_ps(Tolerance);
	// 절대값 만들기 위한 마스크 0x7FFFFFFF
	__m128 AbsMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));

	for (int i = 0; i < 4; i++)
	{
		__m128 A = _mm_load_ps(&M[i][0]);
		__m128 B = _mm_load_ps(&m.M[i][0]);

		__m128 Diff = _mm_sub_ps(A, B);
		__m128 AbsDiff = _mm_and_ps(Diff, AbsMask);

		__m128 Cmp = _mm_cmple_ps(AbsDiff, Tol);
		if (_mm_movemask_ps(Cmp) != 0xF)
		{
			return false;
		}
	}
	return true;
}

inline inline FMatrix FMatrix::Transpose() const
{
	FMatrix result;
	__m128 A0 = _mm_load_ps(&M[0][0]);
	__m128 A1 = _mm_load_ps(&M[1][0]);
	__m128 A2 = _mm_load_ps(&M[2][0]);
	__m128 A3 = _mm_load_ps(&M[3][0]);

	__m128 T0 = _mm_unpacklo_ps(A0, A1);
	__m128 T1 = _mm_unpacklo_ps(A2, A3);
	__m128 T2 = _mm_unpackhi_ps(A0, A1);
	__m128 T3 = _mm_unpackhi_ps(A2, A3);

	_mm_store_ps(&result.M[0][0], _mm_movelh_ps(T0, T1));
	_mm_store_ps(&result.M[1][0], _mm_movehl_ps(T1, T0));
	_mm_store_ps(&result.M[2][0], _mm_movelh_ps(T2, T3));
	_mm_store_ps(&result.M[3][0], _mm_movehl_ps(T3, T2));

	return result;

	//	FMatrix result = {};
//	for (int row = 0; row < 4; ++row) {
//		for (int col = 0; col < 4; ++col) {
//			result.M[row][col] = M[col][row];
//		}
//	}
//	return result;
}

// SIMD 효과 미비- 연산보다는 대각행렬 형태에 데이터 배치가 위주
// M[0][0]이 레지스터에 올라갔다 내려오는것과 0행이 통째로 올라갔다 내려오는 것과 비용차이 없음
inline FMatrix FMatrix::Scale(float n)
{
	FMatrix result = Identity;
	result.M[0][0] = n;
	result.M[1][1] = n;
	result.M[2][2] = n;

	return result;
}

inline FMatrix FMatrix::Scale(const FVector v)
{
	FMatrix result = Identity;
	result.M[0][0] = v.x;
	result.M[1][1] = v.y;
	result.M[2][2] = v.z;

	return result;
}

inline FMatrix FMatrix::RotateX(float degree)
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

inline FMatrix FMatrix::RotateY(float degree)
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

inline FMatrix FMatrix::RotateZ(float degree)
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

// NOTE: Row vector convention. v' = v * M
//	|	1 - 2 (y^2 + z^2)	2xy + 2wz			2xz - 2wy			|
//	|	2xy - 2wz			1 - 2 (x^2 + z^2)	2yz + 2wx			|
//	|	2xz + 2wy			2yz - 2wx			1 - 2 (x^2 + y^2)	|

// TODO:: 오일러 각에서 회전행렬 구하는경우 필수적으로 사용해야 하는 sin cos에 대부분의 병목이 집중되 
// SSE SinCos 테일러 급수 근사 방식 사용해야 함
// 현재 오브젝트의 자체의 회전행렬 업데이트에는 Quat를 사용하므로 나중에 
inline FMatrix FMatrix::Rotate(const FRotator r)
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


// NOTE: Row vector convention. v' = v * M
//	|	1 - 2 (y^2 + z^2)	2xy + 2wz			2xz - 2wy			|
//	|	2xy - 2wz			1 - 2 (x^2 + z^2)	2yz + 2wx			|
//	|	2xz + 2wy			2yz - 2wx			1 - 2 (x^2 + y^2)	|
inline FMatrix FMatrix::Rotate(const FQuat q)
{
	FMatrix result;
	__m128 Quat = _mm_load_ps(&q.x);
	__m128 Quat2 = _mm_add_ps(Quat, Quat);
	// _MM_SHUFFLE  에는 인자가 역순으로 들어감 따라서 레지스터에는 yxxw순서로 들어간다
	// yy2 xx2 xx2 0
	__m128 V1a = _mm_shuffle_ps(Quat, Quat, _MM_SHUFFLE(3, 0, 0, 1));
	__m128 V1b = _mm_shuffle_ps(Quat2, Quat2, _MM_SHUFFLE(3, 0, 0, 1));
	__m128 V1 = _mm_mul_ps(V1a, V1b);
	// zz2 zz2 yy2 0
	__m128 V2a = _mm_shuffle_ps(Quat, Quat, _MM_SHUFFLE(3, 1, 2, 2));
	__m128 V2b = _mm_shuffle_ps(Quat2, Quat2, _MM_SHUFFLE(3, 1, 2, 2));
	__m128 V2 = _mm_mul_ps(V2a, V2b);
	// xy2 yz2 xz2 0
	__m128 V3a = _mm_shuffle_ps(Quat, Quat, _MM_SHUFFLE(3, 0, 1, 0));
	__m128 V3b = _mm_shuffle_ps(Quat2, Quat2, _MM_SHUFFLE(3, 2, 2, 1));
	__m128 V3 = _mm_mul_ps(V3a, V3b);
	// wz2, wx2, wy2, 0
	__m128 V4a = _mm_shuffle_ps(Quat, Quat, _MM_SHUFFLE(3, 3, 3, 3));
	__m128 V4b = _mm_shuffle_ps(Quat2, Quat2, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 V4 = _mm_mul_ps(V4a, V4b);

	const __m128 OneVec = _mm_set1_ps(1.0f);
	__m128 diag = _mm_sub_ps(OneVec, _mm_add_ps(V2, V1));





	return result;
	//FMatrix result = Identity;
	//const float x2 = q.x + q.x;
	//const float y2 = q.y + q.y;
	//const float z2 = q.z + q.z;
	//const float xx2 = q.x * x2;
	//const float yy2 = q.y * y2;
	//const float zz2 = q.z * z2;
	//result.M[0][0] = 1.0f - (yy2 + zz2);
	//result.M[1][1] = 1.0f - (xx2 + zz2);
	//result.M[2][2] = 1.0f - (xx2 + yy2);
	//const float yz2 = q.y * z2;
	//const float wx2 = q.w * x2;
	//result.M[1][2] = yz2 + wx2;
	//result.M[2][1] = yz2 - wx2;
	//const float xy2 = q.x * y2;
	//const float wz2 = q.w * z2;
	//result.M[0][1] = xy2 + wz2;
	//result.M[1][0] = xy2 - wz2;
	//const float xz2 = q.x * z2;
	//const float wy2 = q.w * y2;
	//result.M[0][2] = xz2 - wy2;
	//result.M[2][0] = xz2 + wy2;

	//return result;
}

inline FMatrix FMatrix::Translation(const FVector v)
{
	FMatrix result = Identity;
	result.M[3][0] = v.x;
	result.M[3][1] = v.y;
	result.M[3][2] = v.z;

	return result;
}

inline FVector FMatrix::GetUnitAxis(EAxis Axis) const
{
	const int i = static_cast<int>(Axis);
	return FVector(M[i][0], M[i][1], M[i][2]);
}

inline FVector FMatrix::TransformPosition(const FVector& V) const
{
	return FVector(
		V.x * M[0][0] + V.y * M[1][0] + V.z * M[2][0] + M[3][0],
		V.x * M[0][1] + V.y * M[1][1] + V.z * M[2][1] + M[3][1],
		V.x * M[0][2] + V.y * M[1][2] + V.z * M[2][2] + M[3][2]);
}

inline FVector FMatrix::TransformVector(const FVector& V) const
{
	return FVector(
		V.x * M[0][0] + V.y * M[1][0] + V.z * M[2][0],
		V.x * M[0][1] + V.y * M[1][1] + V.z * M[2][1],
		V.x * M[0][2] + V.y * M[1][2] + V.z * M[2][2]);
}

// MakeMatrix() 결과가 항상 이 형태라 일반 4x4 역행렬이 필요 없다.
//   M = | A 0 |        M^-1 = | A^-1     0 |
//       | t 1 |               | -t*A^-1  1 |
// Transpose() 와 달리 비균등 스케일에도 동작한다.
inline FMatrix FMatrix::Inverse() const
{
	const float C00 = (M[1][1] * M[2][2] - M[1][2] * M[2][1]);
	const float C01 = -(M[1][0] * M[2][2] - M[1][2] * M[2][0]);
	const float C02 = (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

	const float Det = M[0][0] * C00 + M[0][1] * C01 + M[0][2] * C02;
	if (FMath::Abs(Det) < SMALL_NUMBER)
	{
		return FMatrix::Zero;   // 스케일 0 등 역행렬이 없는 경우
	}

	const float C10 = -(M[0][1] * M[2][2] - M[0][2] * M[2][1]);
	const float C11 = (M[0][0] * M[2][2] - M[0][2] * M[2][0]);
	const float C12 = -(M[0][0] * M[2][1] - M[0][1] * M[2][0]);
	const float C20 = (M[0][1] * M[1][2] - M[0][2] * M[1][1]);
	const float C21 = -(M[0][0] * M[1][2] - M[0][2] * M[1][0]);
	const float C22 = (M[0][0] * M[1][1] - M[0][1] * M[1][0]);

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

inline FVector FMatrix::GetTranslation() const
{
	return FVector(M[3][0], M[3][1], M[3][2]);
}

inline FVector FMatrix::GetScale() const
{
	float scaleX = std::sqrt(M[0][0] * M[0][0] + M[1][0] * M[1][0] + M[2][0] * M[2][0]);
	float scaleY = std::sqrt(M[0][1] * M[0][1] + M[1][1] * M[1][1] + M[2][1] * M[2][1]);
	float scaleZ = std::sqrt(M[0][2] * M[0][2] + M[1][2] * M[1][2] + M[2][2] * M[2][2]);
	return FVector(scaleX, scaleY, scaleZ);
}

inline const FMatrix FMatrix::Identity = { {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
} };

inline const FMatrix FMatrix::Zero = { {
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }
} };

inline const FMatrix FMatrix::UEToDX = { {
	{ 0, 0, 1, 0 },
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 0, 1 }
} };
