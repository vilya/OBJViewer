#include "vector.h"

#include <algorithm>
#include <cmath>
#include <cstring>


namespace vh {

  //
  // Vector2
  //

  Vector2::Vector2() :
    x(),
    y()
  {}


  Vector2::Vector2(float ix, float iy) :
    x(ix),
    y(iy)
  {}


  Vector2::Vector2(const Vector2& v) :
    x(v.x),
    y(v.y)
  {}


  //
  // Vector3
  //

  Vector3::Vector3() :
    x(),
    y(),
    z()
  {}


  Vector3::Vector3(float ix, float iy, float iz) :
    x(ix),
    y(iy),
    z(iz)
  {}


  Vector3::Vector3(const Vector2& v, float iz) :
    x(v.x),
    y(v.y),
    z(iz)
  {}


  Vector3::Vector3(const Vector3& v) :
    x(v.x),
    y(v.y),
    z(v.z)
  {}


  //
  // Vector4
  //

  Vector4::Vector4() :
    x(),
    y(),
    z(),
    w()
  {}


  Vector4::Vector4(float ix, float iy, float iz, float iw) :
    x(ix),
    y(iy),
    z(iz),
    w(iw)
  {}


  Vector4::Vector4(const Vector2& v, float iz, float iw) :
    x(v.x),
    y(v.y),
    z(iz),
    w(iw)
  {}


  Vector4::Vector4(const Vector3& v, float iw) :
    x(v.x),
    y(v.y),
    z(v.z),
    w(iw)
  {}


  Vector4::Vector4(const Vector4& v) :
    x(v.x),
    y(v.y),
    z(v.z),
    w(v.w)
  {}


  //
  // Matrix3
  //

  Matrix3::Matrix3()
  {
    memset(data, 0, sizeof(data));
  }


  Matrix3::Matrix3(const Matrix3& m)
  {
    memcpy(data, m.data, sizeof(data));
  }


  Matrix3 Matrix3::identity()
  {
    Matrix3 m;
    m.m00 = m.m11 = m.m22 = 1.0;
    return m;
  }


  Matrix3 Matrix3::rows(const Vector3& a, const Vector3& b, const Vector3& c)
  {
    Matrix3 m;
    
    m.m00 = a.x;
    m.m01 = a.y;
    m.m02 = a.z;

    m.m10 = b.x;
    m.m11 = b.y;
    m.m12 = b.z;

    m.m20 = c.x;
    m.m21 = c.y;
    m.m22 = c.z;

    return m;
  }


  Matrix3 Matrix3::columns(const Vector3& a, const Vector3& b, const Vector3& c)
  {
     Matrix3 m;
    
    m.m00 = a.x;
    m.m10 = a.y;
    m.m20 = a.z;

    m.m01 = b.x;
    m.m11 = b.y;
    m.m21 = b.z;

    m.m02 = c.x;
    m.m12 = c.y;
    m.m22 = c.z;

    return m;
  }


  Matrix3 Matrix3::rotationX(float degrees) 
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix3 m = identity();
    m.m11 = c; m.m12 = -s;
    m.m21 = s; m.m22 = c;
    return m;
  }


  Matrix3 Matrix3::rotationY(float degrees)
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix3 m = identity();
    m.m00 = c; m.m02 = s;
    m.m20 = -s; m.m22 = c;
    return m;
  }


  Matrix3 Matrix3::rotationZ(float degrees)
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix3 m = identity();
    m.m00 = c; m.m01 = -s;
    m.m10 = s; m.m11 = c;
    return m;
  }


  Matrix3 Matrix3::translation(float x, float y)
  {
    Matrix3 m = identity();
    m.m02 = x;
    m.m12 = y;
    return m;
  }


  Matrix3 Matrix3::scale(float sx, float sy, float sz)
  {
    Matrix3 m;
    m.m00 = sx;
    m.m11 = sy;
    m.m22 = sz;
    return m;
  }


  //
  // Matrix4
  //

  Matrix4::Matrix4()
  {
    memset(data, 0, sizeof(data));
  }


  Matrix4::Matrix4(const Matrix3& m)
  {
    m00 = m.m00;
    m01 = m.m01;
    m02 = m.m02;
    m03 = 0.0;

    m10 = m.m10;
    m11 = m.m11;
    m12 = m.m12;
    m13 = 0.0;

    m20 = m.m20;
    m21 = m.m21;
    m22 = m.m22;
    m23 = 0.0;

    m30 = 0.0;
    m31 = 0.0;
    m32 = 0.0;
    m33 = 1.0;
  }


  Matrix4::Matrix4(const Matrix4& m)
  {
    memcpy(data, m.data, sizeof(data));
  }


  Matrix4 Matrix4::identity()
  {
    Matrix4 m;
    m.m00 = m.m11 = m.m22 = m.m33 = 1.0;
    return m;
  }


  Matrix4 Matrix4::rows(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
  {
    Matrix4 m;

    m.m00 = a.x;
    m.m01 = a.y;
    m.m02 = a.z;
    m.m03 = a.w;

    m.m10 = b.x;
    m.m11 = b.y;
    m.m12 = b.z;
    m.m13 = b.w;

    m.m20 = c.x;
    m.m21 = c.y;
    m.m22 = c.z;
    m.m23 = c.w;

    m.m30 = d.x;
    m.m31 = d.y;
    m.m33 = d.z;
    m.m33 = d.w;

    return m;
  }


  Matrix4 Matrix4::columns(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d)
  {
    Matrix4 m;

    m.m00 = a.x;
    m.m10 = a.y;
    m.m20 = a.z;
    m.m30 = a.w;

    m.m01 = b.x;
    m.m11 = b.y;
    m.m21 = b.z;
    m.m31 = b.w;

    m.m02 = c.x;
    m.m12 = c.y;
    m.m22 = c.z;
    m.m32 = c.w;

    m.m03 = d.x;
    m.m13 = d.y;
    m.m33 = d.z;
    m.m33 = d.w;

    return m;
  }


  Matrix4 Matrix4::rotationX(float degrees)
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix4 m = identity();
    m.m11 = c; m.m12 = -s;
    m.m21 = s; m.m22 = c;
    return m;
  }


  Matrix4 Matrix4::rotationY(float degrees)
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix4 m = identity();
    m.m00 = c; m.m02 = s;
    m.m20 = -s; m.m22 = c;
    return m;
  }


  Matrix4 Matrix4::rotationZ(float degrees)
  {
    float radians = degrees * M_PI / 180.0;
    float c = cosf(radians);
    float s = sinf(radians);

    Matrix4 m = identity();
    m.m00 = c; m.m01 = -s;
    m.m10 = s; m.m11 = c;
    return m;
  }


  Matrix4 Matrix4::rotation(float degrees, float x, float y, float z)
  {
    // TODO
    return identity();
  }


  Matrix4 Matrix4::translation(float x, float y, float z)
  {
    Matrix4 m = identity();
    m.m03 = x;
    m.m13 = y;
    m.m23 = z;
    return m;
  }


  Matrix4 Matrix4::scale(float sx, float sy, float sz, float sw)
  {
    Matrix4 m;
    m.m00 = sx;
    m.m11 = sy;
    m.m22 = sz;
    m.m33 = sw;
    return m;
  }


  //
  // Vector2-related Functions
  //

  Vector2 operator + (const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
  }


  Vector2 operator - (const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
  }


  Vector2 operator * (const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(lhs.x * rhs.x, lhs.y * rhs.y);
  }


  Vector2 operator / (const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(lhs.x / rhs.x, lhs.y / rhs.y);
  }


  Vector2 operator * (const Vector2& lhs, float rhs)
  {
    return Vector2(lhs.x * rhs, lhs.y * rhs);
  }


  Vector2 operator * (float lhs, const Vector2& rhs)
  {
    return Vector2(lhs * rhs.x, lhs * rhs.y);
  }


  Vector2 operator / (const Vector2& lhs, float rhs)
  {
    return Vector2(lhs.x / rhs, lhs.y / rhs);
  }


  Vector2 operator / (float lhs, const Vector2& rhs)
  {
    return Vector2(lhs / rhs.x, lhs / rhs.y);
  }


  float sum(const Vector2& v)
  {
    return v.x + v.y;
  }


  float dot(const Vector2& lhs, const Vector2& rhs)
  {
    return sum(lhs * rhs);
  }


  Vector2 pow(const Vector2& v, float exponent)
  {
    return Vector2(powf(v.x, exponent), powf(v.y, exponent));
  }


  float min(const Vector2& v)
  {
    return std::min(v.x, v.y);
  }


  float max(const Vector2& v)
  {
    return std::max(v.x, v.y);
  }


  Vector2 clamp(const Vector2& v, float low, float high)
  {
    return Vector2(std::min(std::max(v.x, low), high),
                   std::min(std::max(v.y, low), high));
  }


  Vector2 lowCorner(const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y));
  }


  Vector2 highCorner(const Vector2& lhs, const Vector2& rhs)
  {
    return Vector2(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y));
  }


  //
  // Vector4-related Functions
  //

  Vector3 operator + (const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
  }


  Vector3 operator - (const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
  }


  Vector3 operator * (const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
  }


  Vector3 operator / (const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
  }


  Vector3 operator * (const Vector3& lhs, float rhs)
  {
    return Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
  }


  Vector3 operator * (float lhs, const Vector3& rhs)
  {
    return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
  }


  Vector3 operator / (const Vector3& lhs, float rhs)
  {
    return Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
  }


  Vector3 operator / (float lhs, const Vector3& rhs)
  {
    return Vector3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
  }


  Vector3 operator * (const Vector3& lhs, const Matrix3& rhs)
  {
    return Vector3(lhs.x * rhs.m00 + lhs.y * rhs.m10 + lhs.z * rhs.m20,
                   lhs.x * rhs.m01 + lhs.y * rhs.m11 + lhs.z * rhs.m21,
                   lhs.x * rhs.m02 + lhs.y * rhs.m12 + lhs.z * rhs.m22);
  }


  Vector3 operator * (const Matrix3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z,
                   lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z,
                   lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z);
  }


  float sum(const Vector3& v)
  {
    return v.x + v.y + v.z;
  }


  float dot(const Vector3& lhs, const Vector3& rhs)
  {
    return sum(lhs * rhs);
  }


  float min(const Vector3& v)
  {
    return std::min(std::min(v.x, v.y), v.z);
  }


  float max(const Vector3& v)
  {
    return std::max(std::max(v.x, v.y), v.z);
  }


  float lengthSqr(const Vector3& v)
  {
    return dot(v, v);
  }


  float length(const Vector3& v)
  {
    return sqrtf(lengthSqr(v));
  }


  Vector3 pow(const Vector3& v, float exponent)
  {
    return Vector3(powf(v.x, exponent), powf(v.y, exponent), powf(v.z, exponent));
  }


  Vector3 cross(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x);
  }


  Vector3 clamp(const Vector3& v, float low, float high)
  {
    return Vector3(std::min(std::max(v.x, low), high),
                   std::min(std::max(v.y, low), high),
                   std::min(std::max(v.z, low), high));
  }


  Vector3 lowCorner(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(std::min(lhs.x, rhs.x),
                   std::min(lhs.y, rhs.y),
                   std::min(lhs.z, rhs.z));
  }


  Vector3 highCorner(const Vector3& lhs, const Vector3& rhs)
  {
    return Vector3(std::max(lhs.x, rhs.x),
                   std::max(lhs.y, rhs.y),
                   std::max(lhs.z, rhs.z));
  }


  Vector3 norm(const Vector3& lhs)
  {
    return lhs / length(lhs);
  }


  //
  // Vector4-related Functions
  //

  Vector4 operator + (const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
  }


  Vector4 operator - (const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
  }


  Vector4 operator * (const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
  }


  Vector4 operator / (const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
  }


  Vector4 operator * (const Vector4& lhs, float rhs)
  {
    return Vector4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
  }


  Vector4 operator * (float lhs, const Vector4& rhs)
  {
    return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
  }


  Vector4 operator / (const Vector4& lhs, float rhs)
  {
    return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
  }


  Vector4 operator / (float lhs, const Vector4& rhs)
  {
    return Vector4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
  }


  Vector4 operator * (const Vector4& lhs, const Matrix4& rhs)
  {
    return Vector4(lhs.x * rhs.m00 + lhs.y * rhs.m10 + lhs.z * rhs.m20 + lhs.w * rhs.m30,
                   lhs.x * rhs.m01 + lhs.y * rhs.m11 + lhs.z * rhs.m21 + lhs.w * rhs.m31,
                   lhs.x * rhs.m02 + lhs.y * rhs.m12 + lhs.z * rhs.m22 + lhs.w * rhs.m32,
                   lhs.x * rhs.m03 + lhs.y * rhs.m13 + lhs.z * rhs.m23 + lhs.w * rhs.m33);
  }


  Vector4 operator * (const Matrix4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z + lhs.m03 * rhs.w,
                   lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z + lhs.m13 * rhs.w,
                   lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z + lhs.m23 * rhs.w,
                   lhs.m30 * rhs.x + lhs.m31 * rhs.y + lhs.m32 * rhs.z + lhs.m33 * rhs.w);
  }


  float sum(const Vector4& v)
  {
    return v.x + v.y + v.z + v.w;
  }


  float dot(const Vector4& lhs, const Vector4& rhs)
  {
    return sum(lhs * rhs);
  }


  float min(const Vector4& v)
  {
    return std::min(std::min(std::min(v.x, v.y), v.z), v.w);
  }


  float max(const Vector4& v)
  {
    return std::max(std::max(std::max(v.x, v.y), v.z), v.w);
  }


  float lengthSqr(const Vector4& v)
  {
    return dot(v, v);
  }


  float length(const Vector4& v)
  {
    return sqrtf(lengthSqr(v));
  }


  Vector4 pow(const Vector4& v, float exponent)
  {
    return Vector4(powf(v.x, exponent), powf(v.y, exponent), powf(v.z, exponent), powf(v.w, exponent));
  }


  Vector4 cross(const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x,
                   1);
  }


  Vector4 clamp(const Vector4& v, float low, float high)
  {
    return Vector4(std::min(std::max(v.x, low), high),
                   std::min(std::max(v.y, low), high),
                   std::min(std::max(v.z, low), high),
                   std::min(std::max(v.w, low), high));
  }


  Vector4 lowCorner(const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(std::min(lhs.x, rhs.x),
                   std::min(lhs.y, rhs.y),
                   std::min(lhs.z, rhs.z),
                   std::min(lhs.w, rhs.w));
  }


  Vector4 highCorner(const Vector4& lhs, const Vector4& rhs)
  {
    return Vector4(std::max(lhs.x, rhs.x),
                   std::max(lhs.y, rhs.y),
                   std::max(lhs.z, rhs.z),
                   std::max(lhs.w, rhs.w));
  }


  Vector4 norm(const Vector4& lhs)
  {
    return lhs / length(lhs);
  }


  //
  // Matrix3-related Functions
  //

  Matrix3 operator + (const Matrix3& lhs, const Matrix3& rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs.data[row][col] + rhs.data[row][col];
    return m;
  }


  Matrix3 operator - (const Matrix3& lhs, const Matrix3& rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs.data[row][col] - rhs.data[row][col];
    return m;
  }


  Matrix3 operator * (const Matrix3& lhs, const Matrix3& rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = dot(rowVec(lhs, row), columnVec(rhs, col));
    return m;
  }


  Matrix3 operator * (const Matrix3& lhs, float rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs.data[row][col] * rhs;
    return m;
  }


  Matrix3 operator * (float lhs, const Matrix3& rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs * rhs.data[row][col];
    return m;
  }


  Matrix3 operator / (const Matrix3& lhs, float rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs.data[row][col] / rhs;
    return m;
  }


  Matrix3 operator / (float lhs, const Matrix3& rhs)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = lhs / rhs.data[row][col];
    return m;
  }


  Matrix3 pow(const Matrix3& lhs, float exponent)
  {
    Matrix3 m;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        m.data[row][col] = powf(lhs.data[row][col], exponent);
    return m;
  }


  float det(const Matrix3& m)
  {
    return 
      m.data[0][0] * (m.data[1][1] * m.data[2][2] - m.data[1][2] * m.data[2][1]) -
      m.data[0][1] * (m.data[1][0] * m.data[2][2] - m.data[1][2] * m.data[2][0]) +
      m.data[0][2] * (m.data[1][0] * m.data[2][1] - m.data[1][1] * m.data[2][0]);
  }


  bool invertible(const Matrix3& m)
  {
    return det(m) == 0.0;
  }


  Matrix3 inverse(const Matrix3& m)
  {
    return m / det(m);
  }


  Vector3 columnVec(const Matrix3& m, unsigned int col)
  {
    return Vector3(m.data[0][col], m.data[1][col], m.data[2][col]);
  }


  Vector3 rowVec(const Matrix3& m, unsigned int row)
  {
    return Vector3(m.data[row][0], m.data[row][1], m.data[row][2]);
  }


  //
  // Matrix4-related Functions
  //

  Matrix4 operator + (const Matrix4& lhs, const Matrix4& rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs.data[row][col] + rhs.data[row][col];
    return m;
  }


  Matrix4 operator - (const Matrix4& lhs, const Matrix4& rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs.data[row][col] - rhs.data[row][col];
    return m;
  }


  Matrix4 operator * (const Matrix4& lhs, const Matrix4& rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = dot(rowVec(lhs, row), columnVec(rhs, col));
    return m;
  }


  Matrix4 operator * (const Matrix4& lhs, float rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs.data[row][col] * rhs;
    return m;
  }


  Matrix4 operator * (float lhs, const Matrix4& rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs * rhs.data[row][col];
    return m;
  }


  Matrix4 operator / (const Matrix4& lhs, float rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs.data[row][col] / rhs;
    return m;
  }


  Matrix4 operator / (float lhs, const Matrix4& rhs)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = lhs / rhs.data[row][col];
    return m;
  }



  Matrix4 pow(const Matrix4& lhs, float exponent)
  {
    Matrix4 m;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col)
        m.data[row][col] = powf(lhs.data[row][col], exponent);
    return m;
  }


  float det(const Matrix4& m)
  {
    // TODO
    return 0.0;
  }


  bool invertible(const Matrix4& m)
  {
    return det(m) == 0.0;
  }


  Matrix4 inverse(const Matrix4& m)
  {
    return m / det(m);
  }
  

  Vector4 columnVec(const Matrix4& m, unsigned int col)
  {
    return Vector4(m.data[0][col], m.data[1][col], m.data[2][col], m.data[3][col]);
  }


  Vector4 rowVec(const Matrix4& m, unsigned int row)
  {
    return Vector4(m.data[row][0], m.data[row][1], m.data[row][2], m.data[row][3]);
  }


} // namespace vh

