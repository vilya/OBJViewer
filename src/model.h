#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>

#include <imagelib.h>
#include "math3d.h"
#include "curve.h"


struct Material {
  Float4 Ka; // Ambient colour
  Float4 Kd; // Diffuse colour
  Float4 Ks; // Specular colour
  Float4 Tf; // Transmission filter.
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


class Model {
public:
  std::vector<Curve> v;
  std::vector<Curve> vt;
  std::vector<Curve> vn;
  std::vector<Curve> colors;
  std::vector<Face*> faces;
  std::map<std::string, Material*> materials;

  Float4 low;
  Float4 high;

  Model();
  ~Model();

  void addV(const Float4& newV);
  void addVt(const Float4& newVt);
  void addVn(const Float4& newVn);
  void addColor(const Float4& newColor);

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

