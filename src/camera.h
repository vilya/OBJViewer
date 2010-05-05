#ifndef OBJViewer_camera_h
#define OBJViewer_camera_h


//#include "math3d.h"
#include "vector.h"


class Camera {
public:
  Camera();

  void moveBy(float x, float y, float z);
  void zoomBy(float amount);
  void rotateByU(float angle);
  void rotateByV(float angle);

  vh::Vector4 getTarget() const;
  vh::Vector4 getRotation() const;
  float getDistance() const;
  float getFieldOfViewY() const;

  void centerView(const vh::Vector4& low, const vh::Vector4& high);
  void frontView(const vh::Vector4& low, const vh::Vector4& high);
  void backView(const vh::Vector4& low, const vh::Vector4& high);
  void leftView(const vh::Vector4& low, const vh::Vector4& high);
  void rightView(const vh::Vector4& low, const vh::Vector4& high);
  void topView(const vh::Vector4& low, const vh::Vector4& high);
  void bottomView(const vh::Vector4& low, const vh::Vector4& high);

  void printCameraInfo() const;

private:
  vh::Vector4 _target;
  vh::Vector4 _rotation; // x,y,z are rotations around that axis; w is the distance.

  float _fieldOfViewY;

  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


#endif // OBJViewer_camera_h

