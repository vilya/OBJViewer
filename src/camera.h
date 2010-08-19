#ifndef OBJViewer_camera_h
#define OBJViewer_camera_h

#include "vgl_arcballcamera.h"
#include "vgl_vec3.h"
#include "vgl_vec4.h"


class Camera : public vgl::ArcballCamera {
public:
  Camera();

  void centerView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void frontView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void backView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void leftView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void rightView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void topView(const vgl::Vec3f& low, const vgl::Vec3f& high);
  void bottomView(const vgl::Vec3f& low, const vgl::Vec3f& high);

protected:
  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


#endif // OBJViewer_camera_h

