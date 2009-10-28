#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <map>
#include <vector>
#include <stdexcept>
#include <string>


const unsigned int _MAX_LINE_LEN = 2048;


class ParseException : public virtual std::exception {
public:
  char message[_MAX_LINE_LEN];

  ParseException(const char *msg_format...);
  virtual ~ParseException() throw() {}
};


struct Float4 {
  union {
    struct { float x, y, z, w; };
    float data[4];
  };

  Float4() : x(0), y(0), z(0), w(1) {}
  Float4(float _x, float _y, float _z, float _w = 1.0) : x(_x), y(_y), z(_z), w(_w) {}
  Float4(const Float4& vec): x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}

  float operator [] (int index) const { return data[index]; }
  float& operator [] (int index) { return data[index]; }
};


struct Material {
  Float4 Ka; // Ambient colour
  Float4 Kd; // Diffuse colour
  Float4 Ks; // Specular colour
  Float4 Tf; // Transmission filter.
  float d;   // Dissolve factor.
  float Ns;  // Specular exponent.

  Material() : Ka(), Kd(), Ks(), Tf(1, 1, 1), d(1.0), Ns(1.0) {}
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


struct Model {
  std::vector<Float4> v;
  std::vector<Float4> vt;
  std::vector<Float4> vp;
  std::vector<Float4> vn;
  std::vector<Face*> faces;

  std::map<std::string, Material> materials;


  Model() : v(), vt(), vp(), vn(), faces(), materials() {}
  ~Model() { for (unsigned int i = 0; i < faces.size(); ++i) delete faces[i]; }
};


Model* loadModel(const char* path) throw(ParseException);

#endif // OBJViewer_model_h

