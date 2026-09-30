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

	__m128 B0 = _mm_loadu_ps(&Other.M[0][0]);
	__m128 B1 = _mm_loadu_ps(&Other.M[1][0]);
	__m128 B2 = _mm_loadu_ps(&Other.M[2][0]);
	__m128 B3 = _mm_loadu_ps(&Other.M[3][0]);

	for (int i = 0; i < 4; i++)
	{
		// A의 i번째 행 가져옴
		__m128 A = _mm_loadu_ps(&M[i][0]);
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

		_mm_storeu_ps(&result.M[i][0], Res);
	}
	return result;
}

inline FMatrix FMatrix::operator*(float Scalar) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(Scalar);

	for (int i = 0; i < 4; i++)
	{
		__m128 A = _mm_loadu_ps(&M[i][0]);
		__m128 Res = _mm_mul_ps(A, B);

		_mm_storeu_ps(&result.M[i][0], Res);
	}

	return result;
}

inline FMatrix FMatrix::operator+ (const FMatrix& Other) const
{
	FMatrix result;

	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_loadu_ps(&M[i][0]);
		__m128 iB = _mm_loadu_ps(&Other.M[i][0]);
		_mm_storeu_ps(&result.M[i][0], _mm_add_ps(iA, iB));
	}
	return result;
}

inline FMatrix FMatrix::operator- (const FMatrix& Other) const
{
	FMatrix result;

	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_loadu_ps(&M[i][0]);
		__m128 iB = _mm_loadu_ps(&Other.M[i][0]);
		_mm_storeu_ps(&result.M[i][0], _mm_sub_ps(iA, iB));
	}
	return result;
}

inline FMatrix FMatrix::operator+(float f) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(f);
	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_loadu_ps(&M[i][0]);
		_mm_storeu_ps(&result.M[i][0], _mm_add_ps(iA, B));
	}
	return result;
}

inline FMatrix FMatrix::operator-(float f) const
{
	FMatrix result;

	__m128 B = _mm_set1_ps(f);
	for (int i = 0; i < 4; i++)
	{
		__m128 iA = _mm_loadu_ps(&M[i][0]);
		_mm_storeu_ps(&result.M[i][0], _mm_sub_ps(iA, B));
	}
	return result;
}

inline bool FMatrix::operator==(const FMatrix& m) const
{
	for (int i = 0; i < 4; i++)
	{
		__m128 A = _mm_loadu_ps(&M[i][0]);
		__m128 B = _mm_loadu_ps(&m.M[i][0]);

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
		__m128 A = _mm_loadu_ps(&M[i][0]);
		__m128 B = _mm_loadu_ps(&m.M[i][0]);

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
	__m128 A0 = _mm_loadu_ps(&M[0][0]);
	__m128 A1 = _mm_loadu_ps(&M[1][0]);
	__m128 A2 = _mm_loadu_ps(&M[2][0]);
	__m128 A3 = _mm_loadu_ps(&M[3][0]);

	__m128 T0 = _mm_unpacklo_ps(A0, A1);
	__m128 T1 = _mm_unpacklo_ps(A2, A3);
	__m128 T2 = _mm_unpackhi_ps(A0, A1);
	__m128 T3 = _mm_unpackhi_ps(A2, A3);

	_mm_storeu_ps(&result.M[0][0], _mm_movelh_ps(T0, T1));
	_mm_storeu_ps(&result.M[1][0], _mm_movehl_ps(T1, T0));
	_mm_storeu_ps(&result.M[2][0], _mm_movelh_ps(T2, T3));
	_mm_storeu_ps(&result.M[3][0], _mm_movehl_ps(T3, T2));

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
inline FMatrix FMatrix::Rotate(const FQuat& q)
{
	FMatrix result;

	const __m128 Constant1110 = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f);
	// x y z w
	__m128 Q0 = _mm_loadu_ps(&q.x); // 안전 위해 비정렬 로드 사용
	// 2x 2y 2z 2w
	__m128 Q1 = _mm_add_ps(Q0, Q0);
	// 2x^2 2y^2 2z^2 2w^2
	__m128 Q2 = _mm_mul_ps(Q0, Q1);

	// 대각 성분 계산
	// 2yy, 2xx, 2xx
	__m128 V0 = _mm_shuffle_ps(Q2, Q2, _MM_SHUFFLE(3, 0, 0, 1));
	// 2zz, 2zz, 2yy
	__m128 V1 = _mm_shuffle_ps(Q2, Q2, _MM_SHUFFLE(3, 1, 2, 2));
	__m128 R0 = _mm_sub_ps(Constant1110, V0);
	// 1 - (yy2 + zz2), 1 - (xx2 + zz2), 1 - (xx2 + yy2), @
	R0 = _mm_sub_ps(R0, V1);
	// 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 비트 마스크 생성
	const __m128 Mask = _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
	// 1 - (yy2 + zz2), 1 - (xx2 + zz2), 1 - (xx2 + yy2), 0.0f
	R0 = _mm_and_ps(R0, Mask);

	// 비대각선 성분 계산
	V0 = _mm_shuffle_ps(Q0, Q0, _MM_SHUFFLE(3, 0, 1, 0));
	V1 = _mm_shuffle_ps(Q1, Q1, _MM_SHUFFLE(3, 2, 2, 1));
	// 2xy 2yz 2xz
	V0 = _mm_mul_ps(V0, V1);

	V1 = _mm_shuffle_ps(Q0, Q0, _MM_SHUFFLE(3, 3, 3, 3));
	__m128 V2 = _mm_shuffle_ps(Q1, Q1, _MM_SHUFFLE(3, 1, 0, 2));
	// 2wz 2wx 2wy
	V1 = _mm_mul_ps(V1, V2);

	// 2xy + 2wz, 2yz + 2wx, 2xz + 2wy
	__m128 R1 = _mm_add_ps(V0, V1);
	// 2xy - 2wz, 2yz - 2wx, 2xz - 2wy
	__m128 R2 = _mm_sub_ps(V0, V1);

	//  2xy + 2wz, 2yz + 2wx, 2xz - 2wy, 2xy - 2wz
	V0 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(0, 2, 1, 0));
	//  0행 1행 비대각 성분 2xy + 2wz, 2xz - 2wy, 2xy - 2wz, 2yz + 2wx
	V0 = _mm_shuffle_ps(V0, V0, _MM_SHUFFLE(1, 3, 2, 0));

	// 2xz + 2wy, 2xz + 2wy, 2yz - 2wx, 2yz - 2wx
	V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(1, 1, 2, 2));
	// 2행 비대각 성분 2xz + 2wy, 2yz - 2wx
	V1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(2, 0, 2, 0));

	// 행 성분 조립
	// 0행 순서 정렬 전 1 - (yy2 + zz2), 0.0f, 2xy + 2wz, 2xz - 2wy
	__m128 Row = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(1, 0, 3, 0));
	// 0행 정렬 1 - (yy2 + zz2), 2xy + 2wz, 2xz - 2wy, 0.0f
	Row = _mm_shuffle_ps(Row, Row, _MM_SHUFFLE(1, 3, 2, 0));
	_mm_storeu_ps(&result.M[0][0], Row);

	// 1행 순서 정렬 전 1 - (xx2 + zz2), 0.0f, 2yz + 2wx, 2xy - 2wz
	Row = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(2, 3, 3, 1));
	// 1행 정렬 2xy - 2wz, 1 - (xx2 + zz2), 2yz + 2wx, 0.0f
	Row = _mm_shuffle_ps(Row, Row, _MM_SHUFFLE(1, 2, 0, 3));
	_mm_storeu_ps(&result.M[1][0], Row);

	// 2행 정렬 2xz + 2wy, 2yz - 2wx	, 1 - (xx2 + yy2), 0.0f
	Row = _mm_shuffle_ps(V1, R0, _MM_SHUFFLE(3, 2, 1, 0));
	_mm_storeu_ps(&result.M[2][0], Row);

	_mm_storeu_ps(&result.M[3][0], _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f));

	return result;
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

// 정점 이동 회전 스케일 적용
inline FVector FMatrix::TransformPosition(const FVector& V) const
{
	return FVector(
		V.x * M[0][0] + V.y * M[1][0] + V.z * M[2][0] + M[3][0],
		V.x * M[0][1] + V.y * M[1][1] + V.z * M[2][1] + M[3][1],
		V.x * M[0][2] + V.y * M[1][2] + V.z * M[2][2] + M[3][2]);
}

// 벡터에 회전 스케일만 적용
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
// TODO:: 현재 Inverse는 마우스 피킹 시 프레임 당 최대 1~2번만 호출됨 따라서 SIMD 적용 우선순위 낮음
// 추후 오브젝트의 M행렬의 역행렬 매 프레임당 대량으로 구해야 할 경우 Ftransform 이용해서 구하는 방식이 더 효율적
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
