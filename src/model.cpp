#include "model.h"


//
// Material METHODS
//

Material::Material() :
    Ka(),
    Kd(),
    Ks(),
    Tf(1, 1, 1),
    d(1.0),
    Ns(1.0),
    mapKa(NULL),
    mapKd(NULL),
    mapKs(NULL)
{
}


//
// Vertex METHODS
//

Vertex::Vertex(int _v, int _vt, int _vn) : v(_v), vt(_vt), vn(_vn)
{
}


//
// Face METHODS
//

Face::Face(Material* m) : material(m), vertexes()
{
}


const Vertex& Face::operator [] (unsigned int index) const
{
  return vertexes[index];
}


Vertex& Face::operator [] (unsigned int index)
{
  return vertexes[index];
}


unsigned int Face::size() const
{
  return vertexes.size();
}


//
// Frame METHODS
//

Frame::Frame() :
    v(), vt(), vp(), vn(), faces(),
    low(10000, 10000, 10000),
    high(-10000, -10000, -10000)
{
}


Frame::~Frame()
{
  for (unsigned int i = 0; i < faces.size(); ++i)
    delete faces[i];
}


void Frame::addV(const Float4& newV)
{
  v.push_back(newV);
  for (unsigned int i = 0; i < 3; ++i) {
    if (newV[i] < low[i])
      low[i] = newV[i];
    if (newV[i] > high[i])
      high[i] = newV[i];
  }
}


//
// Model METHODS
//

Model::Model() : frames(), materials()
{
}

