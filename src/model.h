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


struct Frame {
  std::vector<Float4> v;
  std::vector<Float4> vt;
  std::vector<Float4> vp;
  std::vector<Float4> vn;
  std::vector<Face*> faces;

  Float4 low;
  Float4 high;

  Frame();
  ~Frame();

  void addV(const Float4& newV);
};


struct Model {
  std::vector<Frame> frames;
  std::map<std::string, Material> materials;

  Model();
};


#endif // OBJViewer_model_h

