#pragma once

inline FVector::FVector() : x(0), y(0), z(0) {}
inline FVector::FVector(float n) : x(n), y(n), z(n) {}
inline FVector::FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

inline float& FVector::operator[](int Index)
{
	switch (Index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		return z;
	}
}

inline const float& FVector::operator[](int Index) const
{
	switch (Index)
	{
	case 0:
		return x;
	case 1:
		return y;
	default:
		return z;
	}
}

inline const FVector FVector::operator-(const FVector& other) const
{
	return FVector(x - other.x, y - other.y, z - other.z);
}

inline const FVector FVector::operator+(const FVector& other) const
{
	return FVector(x + other.x, y + other.y, z + other.z);
}

inline FVector& FVector::operator+=(const FVector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;

	//TODO:: 복합 대입 연산자에 참조로 반환 안하면 체이닝 연산 안되는데 혹시 이유 있는지 확인
	return *this;
}

inline FVector& FVector::operator-=(const FVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

inline FVector FVector::operator-() const
{
	return FVector(-x, -y, -z);
}

inline float FVector::dot(const FVector& A, const FVector& B)
{
	return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline FVector FVector::cross(const FVector& A, const FVector& B)
{
	return FVector(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
}

inline float FVector::Length() const
{
	return FMath::Sqrt(x * x + y * y + z * z);
}

inline float FVector::LengthSquared() const
{
	return x * x + y * y + z * z;
}

inline void FVector::Normalize()
{
	float len = Length();
	if (len > KINDA_SMALL_NUMBER)
	{
		x /= len;
		y /= len;
		z /= len;
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
}

inline FVector FVector::GetNormalized() const
{
	float len = Length();
	if (len > KINDA_SMALL_NUMBER)
	{
		return FVector(x / len, y / len, z / len);
	}
	else
	{
		return FVector(0.0f, 0.0f, 0.0f);
	}
}

inline bool FVector::IsNearlyZero(float Tolerance) const
{
	return LengthSquared() < Tolerance;
}

inline FVector FVector::Up()
{
	return FVector(0, 0, 1);
}

inline FVector FVector::Right()
{
	return FVector(0, 1, 0);
}

inline FVector FVector::Forward()
{
	return FVector(1, 0, 0);
}

inline const FVector operator*(const FVector& v, float f)
{
	return FVector(v.x * f, v.y * f, v.z * f);
}

inline const FVector operator*(float f, const FVector& v)
{
	return FVector(v.x * f, v.y * f, v.z * f);
}

inline void FVector::Serialize(FStructuredArchive& archive)
{
	archive << TNamedValue{ "x", x };
	archive << TNamedValue{ "y", y };
	archive << TNamedValue{ "z", z };
}

inline FVector4 FVector4::operator-(const FVector4& other) const
{
	__m128 A = _mm_load_ps(&x);
	__m128 B = _mm_load_ps(&other.x);
	__m128 Res = _mm_sub_ps(A, B);

	FVector4 ret;
	_mm_store_ps(&ret.x, Res);
	return ret;
}

inline FVector4 FVector4::operator+(const FVector4& other) const
{
	__m128 A = _mm_load_ps(&x);
	__m128 B = _mm_load_ps(&other.x);
	__m128 Res = _mm_add_ps(A, B);

	FVector4 ret;
	_mm_store_ps(&ret.x, Res);
	return ret;
}

inline FVector4& FVector4::operator+=(const FVector4& other)
{
	__m128 A = _mm_load_ps(&x);
	__m128 B = _mm_load_ps(&other.x);
	__m128 Res = _mm_add_ps(A, B);

	_mm_store_ps(&x, Res);
	return *this;
}

inline FVector4& FVector4::operator-=(const FVector4& other)
{
	__m128 A = _mm_load_ps(&x);
	__m128 B = _mm_load_ps(&other.x);
	__m128 Res = _mm_sub_ps(A, B);

	_mm_store_ps(&x, Res);
	return *this;
}

inline float FVector4::dot(const FVector4& A, const FVector4& B)
{
	__m128 rA = _mm_load_ps(&A.x);
	__m128 rB = _mm_load_ps(&B.x);
	__m128 Res = _mm_dp_ps(rA, rB, 0xF1);

	return _mm_cvtss_f32(Res);
}

inline float FVector4::LengthSqr() const
{
	return dot(*this, *this);
}

inline float FVector4::Length() const
{
	//return FMath::Sqrt(LengthSqr());
	__m128 rA = _mm_load_ps(&x);
	__m128 Dot = _mm_dp_ps(rA, rA, 0xF1);
	__m128 Len = _mm_sqrt_ss(Dot);

	return _mm_cvtss_f32(Len);
}

inline const FVector4 operator*(const FVector4& v, float f)
{
	return FVector4(v.x * f, v.y * f, v.z * f, v.w * f);
	__m128 A = _mm_load_ps(&v.x);
	__m128 B = _mm_set1_ps(f);
	__m128 Res = _mm_mul_ps(A, B);

	FVector4 ret;
	_mm_store_ps(&ret.x, Res);
	return ret;
}

inline const FVector4 operator*(float f, const FVector4& v)
{
	return FVector4(v.x * f, v.y * f, v.z * f, v.w * f);
	__m128 A = _mm_load_ps(&v.x);
	__m128 B = _mm_set1_ps(f);
	__m128 Res = _mm_mul_ps(A, B);

	FVector4 ret;
	_mm_store_ps(&ret.x, Res);
	return ret;
}

inline void FVector4::Serialize(FStructuredArchive& archive)
{
	archive << TNamedValue{ "x", x };
	archive << TNamedValue{ "y", y };
	archive << TNamedValue{ "z", z };
	archive << TNamedValue{ "w", w };
}

inline FVector2::FVector2(float _x, float _y) : x(_x), y(_y) {}

inline const FVector2 FVector2::operator-(const FVector2& other) const
{
	return FVector2(x - other.x, y - other.y);
}

inline const FVector2 FVector2::operator+(const FVector2& other) const
{
	return FVector2(x + other.x, y + other.y);
}

inline void FVector2::operator+=(const FVector2& other)
{
	x += other.x;
	y += other.y;
}

inline void FVector2::operator-=(const FVector2& other)
{
	x -= other.x;
	y -= other.y;
}

inline float FVector2::dot(const FVector2& A, const FVector2& B)
{
	return A.x * B.x + A.y * B.y;
}

inline void FVector2::Normalize()
{
	float len = Length();
	if (len > KINDA_SMALL_NUMBER)
	{
		x /= len;
		y /= len;
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
	}
}

inline FVector2 FVector2::GetNormalized() const
{
	float len = Length();
	if (len > KINDA_SMALL_NUMBER)
	{
		return FVector2(x / len, y / len);
	}
	else
	{
		return FVector2(0.0f, 0.0f);
	}
}

inline float FVector2::Length() const
{
	return FMath::Sqrt(x * x + y * y);
}

inline float FVector2::LengthSquared() const
{
	return x * x + y * y;
}

inline void FVector2::Serialize(FStructuredArchive& archive)
{
	archive << TNamedValue{ "x", x };
	archive << TNamedValue{ "y", y };
}
