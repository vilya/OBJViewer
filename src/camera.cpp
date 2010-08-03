#include "camera.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>


//
// Camera METHODS
//

Camera::Camera() :
  vgl::Camera(800, 600),
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


vgl::Vec3f Camera::getTarget() const
{
  return _target;
}


vgl::Vec4f Camera::getRotation() const
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


void Camera::centerView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 10.0;
  _rotation = vgl::Vec4f(0, 0, 0, distance);
}


void Camera::frontView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = vgl::Vec4f(0, 0, 0, distance);
}


void Camera::backView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = vgl::Vec4f(0, 180, 0, distance);
}


void Camera::leftView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = vgl::Vec4f(0, 270, 0, distance);
}


void Camera::rightView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = vgl::Vec4f(0, 90, 0, distance);
}


void Camera::topView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = vgl::Vec4f(90, 0, 0, distance);
}


void Camera::bottomView(const vgl::Vec3f& low, const vgl::Vec3f& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = vgl::Vec4f(270, 0, 0, distance);
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



