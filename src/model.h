#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>
#include <stdexcept>
#include <string>

#include "image.h"
#include "math3d.h"

const unsigned int _MAX_LINE_LEN = 2048;


class ParseException : public virtual std::exception {
public:
  char message[_MAX_LINE_LEN];

  ParseException(const char *msg_format...);
  virtual ~ParseException() throw() {}

  virtual const char* what() const throw();
};


struct Material {
  Float4 Ka; // Ambient colour
  Float4 Kd; // Diffuse colour
  Float4 Ks; // Specular colour
  Float4 Tf; // Transmission filter.
  float d;   // Dissolve factor.
  float Ns;  // Specular exponent.

  Image* mapKa; // Ambient texture map.
  Image* mapKd; // Diffuse texture map.
  Image* mapKs; // Specular texture map.
  Image* mapD;  // Dissolve texture map.
  Image* mapBump; // Bump map.

  Material() :
      Ka(), Kd(), Ks(), Tf(1, 1, 1), d(1.0), Ns(1.0),
      mapKa(NULL), mapKd(NULL), mapKs(NULL)
  {}
};


struct Vertex {
  int v, vt, vn;

  Vertex(int _v, int _vt, int _vn) : v(_v), vt(_vt), vn(_vn) {}
};


struct Face {
  Material *material;
  std::vector<Vertex> vertexes;

  Face(Material *m = NULL) : material(m), vertexes() {}

  const Vertex& operator [] (unsigned int index) const { return vertexes[index]; }
  Vertex& operator [] (unsigned int index) { return vertexes[index]; }

  unsigned int size() const { return vertexes.size(); }
};


struct Frame {
  std::vector<Float4> v;
  std::vector<Float4> vt;
  std::vector<Float4> vp;
  std::vector<Float4> vn;
  std::vector<Face*> faces;

  Frame() : v(), vt(), vp(), vn(), faces() {}
  ~Frame() { for (unsigned int i = 0; i < faces.size(); ++i) delete faces[i]; }
};


struct Model {
  std::vector<Frame> frames;
  std::map<std::string, Material> materials;
  unsigned int displayListStart;

  Model() : frames(), materials() {}
};


Model* loadModel(const char* path,
    unsigned int startFrame, unsigned int endFrame) throw(ParseException);

#endif // OBJViewer_model_h

