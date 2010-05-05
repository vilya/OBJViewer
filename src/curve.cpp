#include <algorithm>
#include <cmath>

#include "curve.h"


//
// Curve methods
//

Curve::Curve() : _keyframes()
{
}


vh::Vector4& Curve::operator [] (size_t index)
{
  return _keyframes[index];
}


vh::Vector4 Curve::operator [] (size_t index) const
{
  return _keyframes[index];
}


void Curve::addKeyframe(const vh::Vector4& value)
{
  _keyframes.push_back(value);
}


size_t Curve::numKeyframes() const
{
  return _keyframes.size();
}


vh::Vector4 Curve::valueAt(float time) const
{
  switch (_keyframes.size()) {
    case 0:
      return vh::Vector4();
    case 1:
      return _keyframes[0];
    default:
      break;
  }

  int left = (int)floorf(time);
  int right = (left + 1) % _keyframes.size();
  float t = time - left;
  return _keyframes[left] * (1.0 - t) + _keyframes[right] * t;
}

