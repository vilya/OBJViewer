#include "model.h"


//
// Material METHODS
//

Material::Material() :
    Ka(),
    Kd(),
    Ks(),
    Tf(1, 1, 1, 1),
    d(1.0),
    Ns(1.0),
    mapKa(NULL),
    mapKd(NULL),
    mapKs(NULL),
    mapD(NULL),
    mapBump(NULL)
{
}


//
// Vertex METHODS
//

Vertex::Vertex(int _v, int _vt, int _vn, int _c) : v(_v), vt(_vt), vn(_vn), c(_c)
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
// Model METHODS
//

Model::Model() :
    v(), vt(), vn(), colors(), faces(), materials(),
    low(1e20, 1e20, 1e20),
    high(-1e20, -1e20, -1e20),
    _coordNum(0),
    _texCoordNum(0),
    _normalNum(0),
    _colorNum(0),
    _numKeyframes(0)
{
}


Model::~Model()
{
  for (unsigned int i = 0; i < faces.size(); ++i)
    delete faces[i];
  // TODO: delete materials.
}


void Model::addV(const vgl::Vec3f& newV)
{
  while (_coordNum >= v.size())
    v.push_back(Curve3());
  v[_coordNum].addKeyframe(newV);
  ++_coordNum;

  // TODO: bounding box should be represented as a pair of curves too.
  for (unsigned int i = 0; i < 3; ++i) {
    if (newV.data[i] < low.data[i])
      low.data[i] = newV.data[i];
    if (newV.data[i] > high.data[i])
      high.data[i] = newV.data[i];
  }
}


void Model::addVt(const vgl::Vec2f& newVt)
{
  while (_texCoordNum >= vt.size())
    vt.push_back(Curve2());
  vt[_texCoordNum].addKeyframe(newVt);
  ++_texCoordNum;
}


void Model::addVn(const vgl::Vec3f& newVn)
{
  while (_normalNum >= vn.size())
    vn.push_back(Curve3());
  vn[_normalNum].addKeyframe(newVn);
  ++_normalNum;
}


void Model::addColor(const vgl::Vec4f& newColor)
{
  while (_colorNum >= colors.size())
    colors.push_back(Curve4());
  colors[_colorNum].addKeyframe(newColor);
  ++_colorNum;
}


void Model::addFace(Face* newFace)
{
  faces.push_back(newFace);
}


void Model::addMaterial(const std::string& name, Material* newMaterial)
{
  materials[name] = newMaterial;
}


void Model::newKeyframe()
{
  _coordNum = 0;
  _texCoordNum = 0;
  _normalNum = 0;
  _colorNum = 0;

  ++_numKeyframes;
}


size_t Model::numKeyframes()
{
  return _numKeyframes;
}

