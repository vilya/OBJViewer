#include <algorithm>
#include <cmath>

#include "curve.h"


//
// Curve methods
//

Curve::Curve() : _keyframes()
{
}


Float4& Curve::operator [] (size_t index)
{
  return _keyframes[index];
}


Float4 Curve::operator [] (size_t index) const
{
  return _keyframes[index];
}


void Curve::addKeyframe(const Float4& value)
{
  _keyframes.push_back(value);
}


size_t Curve::numKeyframes() const
{
  return _keyframes.size();
}


Float4 Curve::valueAt(float time) const
{
  if (_keyframes.size() == 0)
    return Float4(0, 0, 0, 0);

  int left = (int)floorf(time);
  int right = (left + 1) % _keyframes.size();
  float t = time - left;
  return _keyframes[left] * (1.0 - t) + _keyframes[right] * t;
}

