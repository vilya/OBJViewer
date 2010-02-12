#ifdef linux
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

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


float Camera::getDistance() const
{
  return _rotation.w;
}


void Camera::setup(int width, int height, const Float4& low, const Float4& high)
{
  // Here we use the model's bounding sphere to calculate good values for
  // the near and far clipping planes.
  
  // Radius of bounding sphere == half distance between opposite bbox corners
  // FIXME: For some reason the radius ends up being too small, so instead I'm using the diameter for now.
  float radius = length(high - low); 
  // v is the vector from our target to the bbox center, but we'll only need
  // the z component (see below) so that's all we calculate.
  float vz = ((high.z + low.z) / 2.0) - _target.z;
  // Since our direction vector d for the camera is 0,0,1 the dot product of v
  // and d is simply v.z. This gives us the distance along our direction vector
  // at which we're level with the bbox center: _rotation.w + vz. Subtract the
  // radius from that and we've got out near clip plane.
  float nearClip = _rotation.w + vz - radius;
  // The far clip plane will always be the near clip plane plus the diameter.
  float farClip = nearClip + 2 * radius;
  // Make sure the near clip plane doesn't end up behind us.
  if (nearClip < 0.01)
    nearClip = 0.01;
  // Make sure the far clip plane doesn't end up behind the near clip plane.
  if (farClip <= nearClip)
    farClip = nearClip + 0.01;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  glLoadIdentity();
  glViewport(0, 0, width, height); // Set the viewport to be the entire window
  gluPerspective(_fieldOfViewY, double(width) / double(height), nearClip, farClip);
}


void Camera::transformTo()
{
  glTranslatef(-_target.x, -_target.y, -_target.z - _rotation.w);
  glRotatef(_rotation.x, 1, 0, 0);
  glRotatef(_rotation.y, 0, 1, 0);
  glRotatef(_rotation.z, 0, 0, 1);
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
  float opposite = fmaxf(highU - lowU, highV - lowV) / 2.0;
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



