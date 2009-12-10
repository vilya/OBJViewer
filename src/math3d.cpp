#include <cmath>
#include <cstring>

#include "math3d.h"


//
// Float4 METHODS
//

Float4::Float4() :
  x(0), y(0), z(0), w(1)
{
}


Float4::Float4(float _x, float _y, float _z, float _w) :
  x(_x), y(_y), z(_z), w(_w)
{
}


Float4::Float4(const Float4& vec) :
  x(vec.x), y(vec.y), z(vec.z), w(vec.w)
{
}


float Float4::operator [] (int index) const
{
  return data[index];
}


float& Float4::operator [] (int index)
{
  return data[index];
}


Float4 operator + (const Float4& a, const Float4& b)
{
  return Float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}


Float4 operator - (const Float4& a, const Float4& b)
{
  return Float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}


Float4 operator / (const Float4& a, float k)
{
  return Float4(a.x / k, a.y / k, a.z / k, a.w);
}


//
// Matrix4 METHODS
//

Matrix4::Matrix4()
{
  memset(data, 0, 16 * sizeof(float));
}


Matrix4 Matrix4::identity()
{
  Matrix4 m;
  m.a00 = m.a11 = m.a22 = m.a33 = 1;
  return m;
}


Matrix4 Matrix4::rotateX(float degrees)
{
  Matrix4 m;
  float c = cosf(degrees * M_PI / 180.0);
  float s = sinf(degrees * M_PI / 180.0);

  m.a00 = 1;
  m.a11 = c; m.a12 = -s;
  m.a21 = s; m.a22 = c;
  m.a33 = 1;

  return m;
}


Matrix4 Matrix4::rotateY(float degrees)
{
  Matrix4 m;
  float c = cosf(degrees * M_PI / 180.0);
  float s = sinf(degrees * M_PI / 180.0);

  m.a00 = c; m.a02 = s;
  m.a11 = 1;
  m.a20 = -s; m.a22 = c;
  m.a33 = 1;

  return m;
}


Matrix4 Matrix4::rotateZ(float degrees)
{
  Matrix4 m;
  float c = cosf(degrees * M_PI / 180.0);
  float s = sinf(degrees * M_PI / 180.0);

  m.a00 = c; m.a01 = -s;
  m.a10 = s; m.a11 = c;
  m.a22 = 1;
  m.a33 = 1;

  return m;
}


//
// FUNCTIONS
//

Float4 operator * (const Float4& v, const Matrix4& m)
{
  Float4 r;
  r.x = v.x * m.a00 + v.y * m.a10 + v.z * m.a20 + v.w * m.a30;
  r.y = v.x * m.a01 + v.y * m.a11 + v.z * m.a21 + v.w * m.a31;
  r.z = v.x * m.a02 + v.y * m.a12 + v.z * m.a22 + v.w * m.a32;
  r.w = v.x * m.a03 + v.y * m.a13 + v.z * m.a23 + v.w * m.a33;
  return r;
}


Float4 operator * (const Matrix4& m, const Float4& v)
{
  Float4 r;
  r.x = v.x * m.a00 + v.y * m.a01 + v.z * m.a02 + v.w * m.a03;
  r.y = v.x * m.a10 + v.y * m.a11 + v.z * m.a12 + v.w * m.a13;
  r.z = v.x * m.a20 + v.y * m.a21 + v.z * m.a22 + v.w * m.a23;
  r.w = v.x * m.a30 + v.y * m.a31 + v.z * m.a32 + v.w * m.a33;
  return r;
}

