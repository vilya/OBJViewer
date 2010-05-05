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

  vh::Vector3 getTarget() const;
  vh::Vector4 getRotation() const;
  float getDistance() const;
  float getFieldOfViewY() const;

  void centerView(const vh::Vector3& low, const vh::Vector3& high);
  void frontView(const vh::Vector3& low, const vh::Vector3& high);
  void backView(const vh::Vector3& low, const vh::Vector3& high);
  void leftView(const vh::Vector3& low, const vh::Vector3& high);
  void rightView(const vh::Vector3& low, const vh::Vector3& high);
  void topView(const vh::Vector3& low, const vh::Vector3& high);
  void bottomView(const vh::Vector3& low, const vh::Vector3& high);

  void printCameraInfo() const;

private:
  vh::Vector3 _target;
  vh::Vector4 _rotation; // x,y,z are rotations around that axis; w is the distance.

  float _fieldOfViewY;

  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


#endif // OBJViewer_camera_h

