#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <list>
#include <map>

#include <imagelib.h>
#include "math3d.h"
#include "model.h"
#include "parser.h"


//
// TYPES
//

enum RenderStyle {
  kLines, kPolygons
};

enum LightType {
  kDirectional, kSpotlight
};

enum RenderGroupType {
  kTriangleGroup, kQuadGroup, kPolygonGroup
};


class FramesPerSecond {
public:
  FramesPerSecond();

  void increment();
  float fps() const;

private:
  int _framesDrawn;
  int _since;
  float _fps;
};


class Camera {
public:
  Camera();

  void moveBy(float x, float y, float z);
  void zoomBy(float amount);
  void rotateByU(float angle);
  void rotateByV(float angle);

  Float4 getTarget() const;
  float getDistance() const;

  void setup(int width, int height, const Float4& low, const Float4& high);
  void transformTo();

  void centerView(const Float4& low, const Float4& high);
  void frontView(const Float4& low, const Float4& high);
  void backView(const Float4& low, const Float4& high);
  void leftView(const Float4& low, const Float4& high);
  void rightView(const Float4& low, const Float4& high);
  void topView(const Float4& low, const Float4& high);
  void bottomView(const Float4& low, const Float4& high);

  void printCameraInfo() const;

private:
  Float4 _target;
  Float4 _rotation; // x,y,z are rotations around that axis; w is the distance.

  float _fieldOfViewY;

  float distanceFrom(float highU, float lowU, float highV, float lowV) const;
};


class RenderGroup {
public:
  RenderGroup(Material* iMaterial, RenderGroupType iType);

  Material* getMaterial() const;

  void add(Model* model, Face* face);
  size_t size() const;

  virtual void prepare();
  virtual void render();

private:
  Material* _material;
  RenderGroupType _type;
  size_t _size;
  bool _hasTexCoords;
  bool _hasNormalCoords;

  // Coords are interleaved in groups of up to 9 elements:
  // - First 4 elements in each group are vertex x, y, z and w.
  // - Next 2 are texture x and y (if _hasTexCoords == true).
  // - Final 3 are normal x, y, z (if _hasNormalCoords == true).
  std::vector<float> *_coords;
  GLuint _coordsID;

  std::vector<unsigned int> *_indexes;
  GLuint _indexesID;
};


class Renderer : public ParserCallbacks {
public:
  Renderer(size_t maxTextureWidth, size_t maxTextureHeight);
  ~Renderer();

  Camera* currentCamera();
  Model* currentModel();

  void setStyle(RenderStyle style);
  void toggleDrawLights();
  void toggleHeadlightType();
  void printGLInfo();

  void render(int width, int height);

  // Parser callbacks
  virtual void beginModel(const char* path);
  virtual void endModel();
  virtual void coordParsed(const Float4& coord);
  virtual void texCoordParsed(const Float4& coord);
  virtual void paramCoordParsed(const Float4& coord) {}
  virtual void normalParsed(const Float4& normal);
  virtual void faceParsed(Face* face);
  virtual void materialParsed(const std::string& name, Material* material);
  virtual void textureParsed(RawImage* texture);

private:
  void prepare();

  void drawModel(Model* theModel, RenderStyle style, std::list<RenderGroup*>& groups);
  void drawDefaultModel(RenderStyle style);
  void setupMaterial(Material* material);
  void setupTexture(GLenum texUnit, RawImage* texture, RawImage*& currentTexture);
  void renderGroup(Model* model, RenderStyle style, const RenderGroup* group);

  void loadTextures(std::list<RenderGroup*>& groups);
  void loadTexture(RawImage* tex, bool isMatte);
  void headlight(GLenum light, const Float4& color);
  void drawFPSCounter(int width, int height, float fps);
  void drawBitmapString(float x, float y, void* font, char* str);

  GLint glGet(GLenum what);

private:
  RenderStyle _style;
  LightType _headlightType;
  bool _drawLights;
  Model* _model;
  Camera* _camera;
  size_t _maxTextureWidth, _maxTextureHeight;
  std::list<RenderGroup*> _renderGroups;
  size_t _transparentGroupsStart;

  RawImage* _currentMapKa;
  RawImage* _currentMapKd;
  RawImage* _currentMapKs;
  RawImage* _currentMapD;

  FramesPerSecond _fps;
};


#endif // OBJViewer_renderer_h

