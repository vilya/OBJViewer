#ifndef OBJViewer_curve_h
#define OBJViewer_curve_h

#include <vector>
#include "math3d.h"


class Curve {
public:
  Curve();

  Float4& operator [] (size_t index);
  Float4 operator [] (size_t index) const;

  void addKeyframe(const Float4& value);
  size_t numKeyframes() const;

  Float4 valueAt(float time) const;

private:
  std::vector<Float4> _keyframes;
};


#endif // OBJViewer_curve_h

