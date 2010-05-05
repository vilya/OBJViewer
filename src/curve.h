#ifndef OBJViewer_curve_h
#define OBJViewer_curve_h

#include <vector>
//#include "math3d.h"
#include "vector.h"


class Curve {
public:
  Curve();

  vh::Vector4& operator [] (size_t index);
  vh::Vector4 operator [] (size_t index) const;

  void addKeyframe(const vh::Vector4& value);
  size_t numKeyframes() const;

  vh::Vector4 valueAt(float time) const;

private:
  std::vector<vh::Vector4> _keyframes;
};


#endif // OBJViewer_curve_h

