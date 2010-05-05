#ifndef OBJViewer_curve_h
#define OBJViewer_curve_h

#include <cmath>
#include <vector>
//#include "math3d.h"
#include "vector.h"


namespace vh {


  // The template parameter, VALUE, must meet two requirements:
  // - it must define the * operator, in the form VALUE * float;
  // - it must define the - operator, in the form VALUE - VALUE.

  template <typename VALUE>
  class Curve {
  public:
    Curve() : _keyframes() {}

    VALUE& operator [] (size_t index)       { return _keyframes[index]; }
    VALUE operator [] (size_t index) const  { return _keyframes[index]; }

    void addKeyframe(const VALUE& value)    { _keyframes.push_back(value); }
    size_t numKeyframes() const             { return _keyframes.size(); }
  
    VALUE valueAt(float time) const
    {
      switch (_keyframes.size()) {
        case 0:
          return VALUE();
        case 1:
          return _keyframes[0];
        default:
          break;
      }
      int left = (int)floorf(time);
      int right = (left + 1) % _keyframes.size();
      float t = time - left;
      return _keyframes[left] * (1.0 - t) + _keyframes[right] * t;
    }

  private:
    std::vector<VALUE> _keyframes;
  };


} // namespace vh


#endif // OBJViewer_curve_h

