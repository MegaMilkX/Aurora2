#ifndef FBX_MATH_H
#define FBX_MATH_H

#include <math.h>
#include <stdint.h>

const double FbxPi = 3.14159265359;

template<typename T>
struct TFbxVector2
{
    union { T x, r; };
    union { T y, g; };
    
    TFbxVector2() : x(0), y(0) {}
    TFbxVector2(T x, T y) : x(x), y(y) {}

    T operator[](const int &i) const {
        return *((&x) + i);
    }
    T& operator[](const int &i) {
        return *((&x) + i);
    }

    T length() const { return sqrt(x*x + y*y); }

    operator TFbxVector2<float>() const {
        TFbxVector2<float> r;
        r.x = static_cast<float>(x);
        r.y = static_cast<float>(y);
        return r;
    }
    operator TFbxVector2<double>() const {
        TFbxVector2<double> r;
        r.x = static_cast<double>(x);
        r.y = static_cast<double>(y);
        return r;
    }
};

template<typename T>
struct TFbxVector3
{
    union { T x, r; };
    union { T y, g; };
    union { T z, b; };
    
    TFbxVector3() : x(0), y(0), z(0) {}
    TFbxVector3(T x, T y, T z) : x(x), y(y), z(z) {}

    T operator[](const int &i) const {
        return *((&x) + i);
    }
    T& operator[](const int &i) {
        return *((&x) + i);
    }

    T length() const { return sqrt(x*x + y*y + z*z); }

    operator TFbxVector3<float>() const {
        TFbxVector3<float> r;
        r.x = static_cast<float>(x);
        r.y = static_cast<float>(y);
        r.z = static_cast<float>(z);
        return r;
    }
    operator TFbxVector3<double>() const {
        TFbxVector3<double> r;
        r.x = static_cast<double>(x);
        r.y = static_cast<double>(y);
        r.z = static_cast<double>(z);
        return r;
    }
};

template<typename T>
struct TFbxVector4
{
    union { T x, r; };
    union { T y, g; };
    union { T z, b; };
    union { T w, a; };
    
    TFbxVector4() : x(0), y(0), z(0), w(0) {}
    TFbxVector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    operator TFbxVector3<T>() const { return TFbxVector3<T>(x, y, z); }

    T operator[](const int &i) const {
        return *((&x) + i);
    }
    T& operator[](const int &i) {
        return *((&x) + i);
    }

    T length() const { return sqrt(x*x + y*y + z*z + w*w); }

    operator TFbxVector4<float>() const {
        TFbxVector4<float> r;
        r.x = static_cast<float>(x);
        r.y = static_cast<float>(y);
        r.z = static_cast<float>(z);
        r.w = static_cast<float>(w);
        return r;
    }
    operator TFbxVector4<double>() const {
        TFbxVector4<double> r;
        r.x = static_cast<double>(x);
        r.y = static_cast<double>(y);
        r.z = static_cast<double>(z);
        r.w = static_cast<double>(w);
        return r;
    }
};

template<typename T>
struct TFbxQuat
{
    T x;
    T y;
    T z;
    T w;

    TFbxQuat<T>& operator=(const TFbxVector4<T>& v) {
        x = v.x; y = v.y; z = v.z; w = v.w;
        return *this;
    }
    
    TFbxQuat() : x(0), y(0), z(0), w(1) {}
    TFbxQuat(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    TFbxQuat(const TFbxVector4<T>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    operator TFbxQuat<double>() const {
        TFbxQuat<double> r;
        r.x = static_cast<double>(x);
        r.y = static_cast<double>(y);
        r.z = static_cast<double>(z);
        r.w = static_cast<double>(w);
        return r;
    }

    operator TFbxQuat<float>() const {
        TFbxQuat<float> r;
        r.x = static_cast<float>(x);
        r.y = static_cast<float>(y);
        r.z = static_cast<float>(z);
        r.w = static_cast<float>(w);
        return r;
    }
};

template<typename T>
struct TFbxMatrix3
{
    TFbxMatrix3(){}
    explicit TFbxMatrix3(T f)
    {
        col[0].x = f;
        col[1].y = f;
        col[2].z = f;
    }
    
    TFbxVector3<T> operator[](const int &i) const {
        return col[i];
    }
    TFbxVector3<T>& operator[](const int &i){
        return col[i];
    }

    operator TFbxMatrix3<float>() const {
        TFbxMatrix3<float> r;
        r[0] = static_cast<TFbxVector3<float>>(col[0]);
        r[1] = static_cast<TFbxVector3<float>>(col[1]);
        r[2] = static_cast<TFbxVector3<float>>(col[2]);
        return r;
    }
    operator TFbxMatrix3<double>() const {
        TFbxMatrix3<double> r;
        r[0] = static_cast<TFbxVector3<double>>(col[0]);
        r[1] = static_cast<TFbxVector3<double>>(col[1]);
        r[2] = static_cast<TFbxVector3<double>>(col[2]);
        return r;
    }
private:
    TFbxVector3<T> col[3];
};

template<typename T>
struct TFbxMatrix4
{
    TFbxMatrix4(){}
    explicit TFbxMatrix4(T f)
    {
        col[0].x = f;
        col[1].y = f;
        col[2].z = f;
        col[3].w = f;
    }

    void operator=(const TFbxMatrix3<T>& m)
    {
        (*this) = TFbxMatrix4<T>(static_cast<T>(1));
        col[0][0] = m[0][0]; col[0][1] = m[0][1]; col[0][2] = m[0][2];
        col[1][0] = m[1][0]; col[1][1] = m[1][1]; col[1][2] = m[1][2];
        col[2][0] = m[2][0]; col[2][1] = m[2][1]; col[2][2] = m[2][2];
    }
    
    TFbxVector4<T> operator[](const int &i) const {
        return col[i];
    }
    TFbxVector4<T>& operator[](const int &i){
        return col[i];
    }

    operator TFbxMatrix4<float>() const {
        TFbxMatrix4<float> r;
        r[0] = static_cast<TFbxVector4<float>>(col[0]);
        r[1] = static_cast<TFbxVector4<float>>(col[1]);
        r[2] = static_cast<TFbxVector4<float>>(col[2]);
        r[3] = static_cast<TFbxVector4<float>>(col[3]);
        return r;
    }
    operator TFbxMatrix4<double>() const {
        TFbxMatrix4<double> r;
        r[0] = static_cast<TFbxVector4<double>>(col[0]);
        r[1] = static_cast<TFbxVector4<double>>(col[1]);
        r[2] = static_cast<TFbxVector4<double>>(col[2]);
        r[3] = static_cast<TFbxVector4<double>>(col[3]);
        return r;
    }
private:
    TFbxVector4<T> col[4];
};

typedef TFbxVector2<float> FbxVector2;
typedef TFbxVector2<int64_t> FbxIVector2;
typedef TFbxVector2<double> FbxDVector2;
typedef TFbxVector3<float> FbxVector3;
typedef TFbxVector3<int64_t> FbxIVector3;
typedef TFbxVector3<double> FbxDVector3;
typedef TFbxVector4<float> FbxVector4;
typedef TFbxVector4<int64_t> FbxIVector4;
typedef TFbxVector4<double> FbxDVector4;
typedef TFbxQuat<float> FbxQuat;
typedef TFbxQuat<double> FbxDQuat;
typedef TFbxMatrix3<float> FbxMatrix3;
typedef TFbxMatrix3<double> FbxDMatrix3;
typedef TFbxMatrix4<float> FbxMatrix4;
typedef TFbxMatrix4<double> FbxDMatrix4;

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

template<typename V, typename T>
inline TFbxVector3<V> operator*(const TFbxVector3<V>& a, T f){
    return TFbxVector3<V>(a.x * f, a.y * f, a.z * f);
}
template<typename V, typename T>
inline TFbxVector3<V> operator/(const TFbxVector3<V>& a, T f){
    return TFbxVector3<V>(a.x / f, a.y / f, a.z / f);
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
