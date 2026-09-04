#pragma once

// Structure for a 3D vector
typedef struct FVector
{
    float x, y, z;
    FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    const FVector operator-(const FVector& Others) const
    {
        return FVector(x - Others.x, y - Others.y, z - Others.z);
    }

    void operator+=(const FVector& Others)
    {
        x += Others.x;
        y += Others.y;
        z += Others.z;
    }

    void operator-=(const FVector& Others)
    {
        x -= Others.x;
        y -= Others.y;
        z -= Others.z;
    }

    static float dot(const FVector& A, const FVector& B)
    {
        return A.x * B.x + A.y * B.y + A.z * B.z;
    }
} FVector3;

inline const FVector operator*(const FVector& v, float f)
{
    return FVector(v.x * f, v.y * f, v.z * f);
}

inline const FVector operator*(float f, const FVector& v)
{
    return FVector(v.x * f, v.y * f, v.z * f);
}
