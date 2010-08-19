#include "camera.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>


//
// Camera METHODS
//

Camera::Camera() :
  vgl::ArcballCamera(
      vgl::Vec3f(0, 0, 5), vgl::Vec3f(0, 0, 0), vgl::Vec3f(0, 1, 0),
      -1, 1, -1, 1, 25,
      800, 600)
{
}


void Camera::centerView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.z - low.z) / 10.0f;
  _pos = _target + vgl::Vec3f(0, 0, distance);
  _up = vgl::Vec3f(0, 1, 0);
}


void Camera::frontView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.z - low.z) / 2.0f +
      distanceFrom(high.x, low.x, high.y, low.y);
  _pos = _target + vgl::Vec3f(0, 0, distance);
  _up = vgl::Vec3f(0, 1, 0);
}


void Camera::backView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.z - low.z) / 2.0f +
      distanceFrom(high.x, low.x, high.y, low.y);
  _pos = _target + vgl::Vec3f(0, 0, -distance);
  _up = vgl::Vec3f(0, 1, 0);
}


void Camera::leftView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.x - low.x) / 2.0f +
      distanceFrom(high.z, low.z, high.y, low.y);
  _pos = _target + vgl::Vec3f(-distance, 0, 0);
  _up = vgl::Vec3f(0, 1, 0);
}


void Camera::rightView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.x - low.x) / 2.0f +
      distanceFrom(high.z, low.z, high.y, low.y);
  _pos = _target + vgl::Vec3f(distance, 0, 0);
  _up = vgl::Vec3f(0, 1, 0);
}


void Camera::topView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _pos = _target + vgl::Vec3f(0, distance, 0);
  _up = vgl::Vec3f(0, 0, -1);
}


void Camera::bottomView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2.0f;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _pos = _target + vgl::Vec3f(0, -distance, 0);
  _up = vgl::Vec3f(0, 0, 1);
}


float Camera::distanceFrom(float highU, float lowU, float highV, float lowV) const
{
  float opposite = std::max(highU - lowU, highV - lowV) / 2.0;
  float angle = (_aperture / 2.0) * M_PI / 180.0;
  float adjacent = (opposite / tanf(angle));
  return adjacent;
}

