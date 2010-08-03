#ifndef OBJViewer_camera_h
#define OBJViewer_camera_h

#include "vgl_camera.h"
#include "vgl_vec3.h"
#include "vgl_vec4.h"


class Camera : public vgl::Camera {
public:
  Camera();

  virtual void moveBy(float dx, float dy, float dz);
  virtual void zoomBy(float dz);
  void rotateByU(float angle);
  void rotateByV(float angle);

  vgl::Vec3f getTarget() const;
  vgl::Vec4f getRotation() const;
  float getDistance() const;
  float getFieldOfViewY() const;

  void centerView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void frontView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void backView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void leftView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void rightView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void topView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void bottomView(const vgl::Vec3f& low, const vgl::Vec3f& high);

  void printCameraInfo() const;

private:
  vgl::Vec3f _target;
  vgl::Vec4f _rotation; // x,y,z are rotations around that axis; w is the distance.

  float _fieldOfViewY;

  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


#endif // OBJViewer_camera_h

