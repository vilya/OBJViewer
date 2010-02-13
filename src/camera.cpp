#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "camera.h"


//
// Camera METHODS
//

Camera::Camera() :
  _target(0, 0, 0),
  _rotation(0, 0, 0, 10),
  _fieldOfViewY(30)
{
}


void Camera::moveBy(float x, float y, float z)
{
  _target.x += x;
  _target.y += y;
  _target.z += z;
}


void Camera::zoomBy(float amount)
{
  _rotation.w *= amount;
}


void Camera::rotateByU(float angle)
{
  _rotation.x += angle;
}


void Camera::rotateByV(float angle)
{
  _rotation.y += angle;
}


Float4 Camera::getTarget() const
{
  return _target;
}


Float4 Camera::getRotation() const
{
  return _rotation;
}


float Camera::getDistance() const
{
  return _rotation.w;
}


float Camera::getFieldOfViewY() const
{
  return _fieldOfViewY;
}


void Camera::centerView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 10.0;
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::frontView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::backView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = Float4(0, 180, 0, distance);
}


void Camera::leftView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = Float4(0, 270, 0, distance);
}


void Camera::rightView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = Float4(0, 90, 0, distance);
}


void Camera::topView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = Float4(90, 0, 0, distance);
}


void Camera::bottomView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = Float4(270, 0, 0, distance);
}


float Camera::distanceFrom(float highU, float lowU, float highV, float lowV) const
{
  float opposite = std::max(highU - lowU, highV - lowV) / 2.0;
  float angle = (_fieldOfViewY / 2.0) * M_PI / 180.0;
  float adjacent = (opposite / tanf(angle));
  return adjacent;
}


void Camera::printCameraInfo() const
{
  fprintf(stderr, "Camera at:\n");
  fprintf(stderr, "target = { %f, %f, %f }\n", _target.x, _target.y, _target.z);
  fprintf(stderr, "rotation = { %f, %f, %f, %f }\n",
                  _rotation.x, _rotation.y, _rotation.z, _rotation.w);
}


