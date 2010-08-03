#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>

#include "vgl_image.h"
#include "vgl_vec2.h"
#include "vgl_vec3.h"
#include "vgl_vec4.h"

#include "curve.h"


struct Material {
  vgl::Vec4f Ka; // Ambient colour
  vgl::Vec4f Kd; // Diffuse colour
  vgl::Vec4f Ks; // Specular colour
  vgl::Vec4f Tf; // Transmission filter.
  float d;   // Dissolve factor.
  float Ns;  // Specular exponent.

  vgl::RawImage* mapKa; // Ambient texture map.
  vgl::RawImage* mapKd; // Diffuse texture map.
  vgl::RawImage* mapKs; // Specular texture map.
  vgl::RawImage* mapD;  // Dissolve texture map.
  vgl::RawImage* mapBump; // Bump map.

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


typedef vh::Curve<vgl::Vec2f> Curve2;
typedef vh::Curve<vgl::Vec3f> Curve3;
typedef vh::Curve<vgl::Vec4f> Curve4;


class Model {
public:
  std::vector<Curve3> v;
  std::vector<Curve2> vt;
  std::vector<Curve3> vn;
  std::vector<Curve4> colors;
  std::vector<Face*> faces;
  std::map<std::string, Material*> materials;

  vgl::Vec3f low;
  vgl::Vec3f high;

  Model();
  ~Model();

  void addV(const vgl::Vec3f& newV);
  void addVt(const vgl::Vec2f& newVt);
  void addVn(const vgl::Vec3f& newVn);
  void addColor(const vgl::Vec4f& newColor);

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

