#ifndef FBX_MATH_H
#define FBX_MATH_H

#include <math.h>

const double FbxPi = 3.14159265359;

struct FbxVector2
{
    FbxVector2()
    : FbxVector2(0.0f, 0.0f) {}
    FbxVector2(float x, float y)
    : x(x), y(y) {}
    float operator[](int i) const {
        return *((&x) + i);
    }
    float& operator[](int i) {
        return *((&x) + i);
    }
    float x, y;
};

struct FbxVector3
{
    FbxVector3()
    : FbxVector3(0.0f, 0.0f, 0.0f) {}
    FbxVector3(float x, float y, float z)
    : x(x), y(y), z(z) {}
    float operator[](int i) const {
        return *((&x) + i);
    }
    float& operator[](int i) {
        return *((&x) + i);
    }
    float x, y, z;
};

struct FbxVector4
{
    FbxVector4()
    : FbxVector4(0.0f, 0.0f, 0.0f, 0.0f) {}
    FbxVector4(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w) {}
    const float operator[](int i) const {
        return *((&x) + i);
    }
    float& operator[](int i) {
        return *((&x) + i);
    }
    float x, y, z, w;
};

struct FbxQuat
{
    FbxQuat()
    : FbxQuat(0.0f, 0.0f, 0.0f, 1.0f) {}
    FbxQuat(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w) {}
    float x, y, z, w;  
};

struct FbxMatrix3
{
    FbxVector3& operator[](int col) { return columns[col]; }
    const FbxVector3& operator[](int col) const { return columns[col]; }
    FbxVector3 columns[3];
};

struct FbxMatrix4
{
    FbxMatrix4()
    : FbxMatrix4(1.0f) {}
    FbxMatrix4(float f)
    {
        columns[0].x = f;
        columns[1].y = f;
        columns[2].z = f;
        columns[3].w = f;
    }
    void operator=(const FbxMatrix3& m)
    {
        (*this) = FbxMatrix4(1.0f);
        columns[0][0] = m[0][0]; columns[0][1] = m[0][1]; columns[0][2] = m[0][2];
        columns[1][0] = m[1][0]; columns[1][1] = m[1][1]; columns[1][2] = m[1][2];
        columns[2][0] = m[2][0]; columns[2][1] = m[2][1]; columns[2][2] = m[2][2];
    }
    FbxVector4& operator[](int col) { return columns[col]; }
    const FbxVector4& operator[](int col) const { return columns[col]; }
    FbxVector4 columns[4];
};

inline FbxQuat FbxNormalize(const FbxQuat& a);

inline float FbxQRSqrt(const float &n)
{
    long i;
    float x2, y;
    const float threehalves = 1.5f;
    x2 = n * 0.5f;
    y = n;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalves - (x2 * y * y));
    return y;
}

inline float FbxSqrt(const float &n)
{
    return n * FbxQRSqrt(n);
}

inline float FbxLength(const FbxQuat& q) { return FbxSqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w); }

inline FbxVector4 operator+(const FbxVector4& a, const FbxVector4& b){
    return FbxVector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline FbxVector3 operator*(const FbxVector3& a, float f){
    return FbxVector3(a.x * f, a.y * f, a.z * f);
}

inline FbxVector4 operator*(const FbxVector4& a, float f){
    return FbxVector4(a.x * f, a.y * f, a.z * f, a.w * f);
}

inline FbxQuat operator/(const FbxQuat& a, float f) {
    return FbxQuat(a.x / f, a.y / f, a.z / f, a.w / f);
}

inline FbxQuat operator*(const FbxQuat& q0, const FbxQuat& q1)
{
    return FbxNormalize(FbxQuat((q0.w * q1.x + q1.w * q0.x) + (q0.y * q1.z - q1.y * q0.z),
        (q0.w * q1.y + q1.w * q0.y) + (q1.x * q0.z - q0.x * q1.z), //Inverted, since y axis rotation is inverted
        (q0.w * q1.z + q1.w * q0.z) + (q0.x * q1.y - q1.x * q0.y),
        (q1.w * q0.w) - (q1.x * q0.x) - (q1.y * q0.y) - (q1.z * q0.z)));
}

inline FbxMatrix4 operator*(const FbxMatrix4& m0, const FbxMatrix4& m1) {
    FbxMatrix4 m(0.0f);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                m[i][j] += m0[k][j] * m1[i][k];
    return m;
}

inline FbxQuat FbxNormalize(const FbxQuat& a) 
{
    float l = FbxLength(a);
    if(l == 0.0f)
        return a;
    return a / l;
}

inline FbxMatrix4 FbxTranslate(const FbxMatrix4& m4, const FbxVector3& v3)
{
    FbxMatrix4 r = m4;
        r[3] = m4[0] * v3[0] + m4[1] * v3[1] + m4[2] * v3[2] + m4[3];
    return r;
}

inline FbxMatrix3 FbxToMat3(const FbxQuat& q)
{
    FbxMatrix3 m;
    m[0].x = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
    m[0].y = q.z * 2 * q.w + 2 * q.x * q.y;
    m[0].z = -q.y * 2 * q.w + 2 * q.x * q.z;
    m[1].x = -q.z * 2 * q.w + 2 * q.x * q.y;
    m[1].y = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
    m[1].z = q.x * 2 * q.w + 2 * q.y * q.z;
    m[2].x = q.y * 2 * q.w + 2 * q.x * q.z;
    m[2].y = -q.x * 2 * q.w + 2 * q.y * q.z;
    m[2].z = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
    return m;
}

inline FbxMatrix4 FbxToMatrix4(const FbxQuat& q)
{
    FbxMatrix4 m(1.0f);
    m = FbxToMat3(q);
    return m;
}

inline FbxMatrix4 FbxScale(const FbxMatrix4& m4, const FbxVector3& v3)
{
    FbxMatrix4 r = m4;
    r[0] = r[0] * v3[0];
    r[1] = r[1] * v3[1];
    r[2] = r[2] * v3[2];
    return r;
}

inline FbxQuat FbxAngleAxis(float a, const FbxVector3& axis)
{
    float s = sinf(a * 0.5f);
    return FbxNormalize(FbxQuat(axis.x * s, axis.y * s, axis.z * s, cosf(a*0.5f)));
}

inline FbxQuat FbxEulerToQuat(const FbxVector3& euler)
{
    FbxQuat qx = FbxAngleAxis(euler.x, FbxVector3(1.0f, 0.0f, 0.0f));
    FbxQuat qy = FbxAngleAxis(euler.y, FbxVector3(0.0f, 1.0f, 0.0f));
    FbxQuat qz = FbxAngleAxis(euler.z, FbxVector3(0.0f, 0.0f, 1.0f));
    return FbxNormalize(qz * qy * qx);
}

inline FbxMatrix4 FbxInverse(const FbxMatrix4 &mat)
{
    const float* m;
    m = (float*)&mat;
    float det;
    int i;
    FbxMatrix4 inverse(1.0f);
    float* inv = (float*)&inverse;
    
    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    //assert(det != 0);

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        inv[i] = inv[i] * det;

    return inverse;
}

#endif
