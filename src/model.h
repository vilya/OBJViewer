#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>

#include <imagelib.h>
//#include "math3d.h"
#include "vector.h"
#include "curve.h"


struct Material {
  vh::Vector4 Ka; // Ambient colour
  vh::Vector4 Kd; // Diffuse colour
  vh::Vector4 Ks; // Specular colour
  vh::Vector4 Tf; // Transmission filter.
  float d;   // Dissolve factor.
  float Ns;  // Specular exponent.

  RawImage* mapKa; // Ambient texture map.
  RawImage* mapKd; // Diffuse texture map.
  RawImage* mapKs; // Specular texture map.
  RawImage* mapD;  // Dissolve texture map.
  RawImage* mapBump; // Bump map.

  Material();
};


struct Vertex {
  int v, vt, vn, c;

  Vertex(int _v, int _vt, int _vn, int _c);
};


struct Face {
  Material *material;
  std::vector<Vertex> vertexes;

  Face(Material *m = NULL);

  const Vertex& operator [] (unsigned int index) const;
  Vertex& operator [] (unsigned int index);

  unsigned int size() const;
};


typedef vh::Curve<vh::Vector2> Curve2;
typedef vh::Curve<vh::Vector3> Curve3;
typedef vh::Curve<vh::Vector4> Curve4;


class Model {
public:
  std::vector<Curve3> v;
  std::vector<Curve2> vt;
  std::vector<Curve3> vn;
  std::vector<Curve4> colors;
  std::vector<Face*> faces;
  std::map<std::string, Material*> materials;

  vh::Vector3 low;
  vh::Vector3 high;

  Model();
  ~Model();

  void addV(const vh::Vector3& newV);
  void addVt(const vh::Vector2& newVt);
  void addVn(const vh::Vector3& newVn);
  void addColor(const vh::Vector4& newColor);

  void addFace(Face* newFace);
  void addMaterial(const std::string& name, Material* newMaterial);

  void newKeyframe();
  size_t numKeyframes();

private:
  size_t _coordNum;
  size_t _texCoordNum;
  size_t _normalNum;
  size_t _colorNum;

  size_t _numKeyframes;
};


#endif // OBJViewer_model_h

