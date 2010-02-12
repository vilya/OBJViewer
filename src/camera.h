#ifndef OBJViewer_camera_h
#define OBJViewer_camera_h


#include "math3d.h"


class Camera {
public:
  Camera();

  void moveBy(float x, float y, float z);
  void zoomBy(float amount);
  void rotateByU(float angle);
  void rotateByV(float angle);

  Float4 getTarget() const;
  float getDistance() const;

  void setup(int width, int height, const Float4& low, const Float4& high);
  void transformTo();

  void centerView(const Float4& low, const Float4& high);
  void frontView(const Float4& low, const Float4& high);
  void backView(const Float4& low, const Float4& high);
  void leftView(const Float4& low, const Float4& high);
  void rightView(const Float4& low, const Float4& high);
  void topView(const Float4& low, const Float4& high);
  void bottomView(const Float4& low, const Float4& high);

  void printCameraInfo() const;

private:
  Float4 _target;
  Float4 _rotation; // x,y,z are rotations around that axis; w is the distance.

  float _fieldOfViewY;

  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


#endif // OBJViewer_camera_h

