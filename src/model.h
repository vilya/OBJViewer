#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>

#include <imagelib.h>
#include "math3d.h"


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
  int v, vt, vn;

  Vertex(int _v, int _vt, int _vn);
};


struct Face {
  Material *material;
  std::vector<Vertex> vertexes;

  Face(Material *m = NULL);

  const Vertex& operator [] (unsigned int index) const;
  Vertex& operator [] (unsigned int index);

  unsigned int size() const;
};


struct Model {
  std::vector<Float4> v;
  std::vector<Float4> vt;
  std::vector<Float4> vp;
  std::vector<Float4> vn;
  std::vector<Face*> faces;
  std::map<std::string, Material> materials;

  Float4 low;
  Float4 high;

  Model();
  ~Model();
  void addV(const Float4& newV);
};


#endif // OBJViewer_model_h

