#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <list>
#include <map>

#include <imagelib.h>
#include "math3d.h"
#include "model.h"
#include "parser.h"
#include "camera.h"


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
  kTriangleGroup, kPolygonGroup
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




class RenderGroup {
public:
  RenderGroup(Material* iMaterial, RenderGroupType iType, GLuint defaultTextureID);

  Material* getMaterial() const;

  void add(Model* model, Face* face);
  size_t size() const;

  virtual void prepare();
  virtual void render();
  virtual void renderPoints();
  virtual void renderLines();

private:
  Material* _material;
  RenderGroupType _type;
  size_t _size;
  bool _hasTexCoords;
  bool _hasNormalCoords;
  bool _hasColors;

  // Coords are interleaved in groups of up to 12 elements:
  // - First 3 elements in each group are vertex x, y, z.
  // - Next 2 are texture u and v (if _hasTexCoords == true).
  // - Next 4 are normal x, y, z and w (if _hasNormalCoords == true).
  // - Final 3 are color r, g and b (if _hasColors == true).
  std::vector<float> _coords;
  GLuint _coordsID;

  std::vector<unsigned int> _indexes;
  GLuint _indexesID;

  GLuint _defaultTextureID;
};


class Renderer : public ParserCallbacks {
public:
  Renderer(Camera* camera, size_t maxTextureWidth, size_t maxTextureHeight);
  ~Renderer();

  Camera* currentCamera();
  Model* currentModel();

  void setStyle(RenderStyle style);
  void toggleDrawPolys();
  void toggleDrawPoints();
  void toggleDrawLines();
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
  virtual void colorParsed(const Float4& color);
  virtual void faceParsed(Face* face);
  virtual void materialParsed(const std::string& name, Material* material);
  virtual void textureParsed(RawImage* texture);

private:
  void prepare();
  void prepareModel();
  void prepareRenderGroups();
  void prepareMaterials();
  void prepareShaders();

  void drawModel(Model* theModel, RenderStyle style, std::list<RenderGroup*>& groups);
  void drawDefaultModel();

  void loadTextures(std::list<RenderGroup*>& groups);
  void loadTexture(RawImage* tex, bool isMatte);
  void headlight(GLenum light, const Float4& color);
  void drawHUD(int width, int height, float fps);
  void drawBitmapString(float x, float y, void* font, char* str);

  const char* loadShader(const char* path);
  GLint glGet(GLenum what);

  void printShaderInfoLog(GLuint obj);
  void printProgramInfoLog(GLuint obj);

private:
  RenderStyle _style;
  LightType _headlightType;
  bool _drawPolys, _drawPoints, _drawLines;
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

  RawImage* _defaultTexture;
  GLuint _programObject;
  
};


#endif // OBJViewer_renderer_h

