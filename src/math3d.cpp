#include <cmath>

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

