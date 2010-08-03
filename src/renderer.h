#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <list>
#include <map>

#include "vgl.h"

#include "model.h"
//#include "camera.h"
#include "resources.h"


//
// TYPES
//

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
  RenderGroup(Material* iMaterial, RenderGroupType iType, GLuint iShaderProgramID);

  Material* getMaterial() const;

  void add(Model* model, Face* face);
  size_t size() const;

  size_t floatsPerVertex() const;
  void flipNormals();

  void prepare();
  void render(float time);
  void renderPoints(float time);
  void renderLines(float time);

private:
  void setTime(float time);
  void setupShaders();

private:
  Material* _material;
  RenderGroupType _type;
  size_t _size;
  bool _hasColors;
  float _currentTime;
  bool _flipNormals;

  // Coords are interleaved in groups of up to 12 elements:
  // - First 3 elements in each group are vertex x, y, z.
  // - Next 2 are texture u and v (if _hasTexCoords == true).
  // - Next 4 are normal x, y, z and w (if _hasNormalCoords == true).
  // - Final 3 are color r, g and b (if _hasColors == true).
  std::vector<Curve3*> _coords;
  std::vector<Curve2*> _texCoords;
  std::vector<Curve3*> _normals;
  std::vector<Curve4*> _colors;
  GLuint _bufferID;
  GLuint _indexesID;

  GLuint _shaderProgramID;
};


class Renderer : public vgl::Renderer {
public:
  Renderer(ResourceManager* resources, Model* model,
      size_t maxTextureWidth, size_t maxTextureHeight, float animFPS);
  ~Renderer();

  Model* currentModel();

  void toggleDrawPolys();
  void toggleDrawPoints();
  void toggleDrawLines();
  void toggleHeadlightType();
  void printGLInfo();

  virtual void setup();
  virtual void render();

  void setTime(float time);
  void nextFrame();
  void previousFrame();
  void firstFrame();
  void lastFrame();
  void togglePlaying();

  void flipNormals();

private:
  void prepareModel();
  void prepareRenderGroups();
  void prepareMaterials();
  void prepareShaders();

  void drawModel(Model* theModel, std::list<RenderGroup*>& groups);
  void drawDefaultModel();

  void loadTextures(std::list<RenderGroup*>& groups);
  void loadTexture(vgl::RawImage* tex, bool isMatte);
  void headlight(GLenum light, const vgl::Vec4f& color);
  void drawHUD();
  void drawBitmapString(float x, float y, void* font, char* str);
  void drawRightAlignedBitmapString(float x, float y, void* font, char* str);

  GLuint loadShader(GLenum shaderType, const std::string& path);
  GLuint linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
  GLint glGet(GLenum what);

  void printShaderInfoLog(GLuint obj);
  void printProgramInfoLog(GLuint obj);

  float calculatePlaybackTime();

private:
  LightType _headlightType;
  bool _drawPolys, _drawPoints, _drawLines;
  ResourceManager* _resources;
  Model* _model;
  float _animFPS;
  size_t _maxTextureWidth, _maxTextureHeight;
  std::list<RenderGroup*> _renderGroups;
  size_t _transparentGroupsStart;

  vgl::RawImage* _currentMapKa;
  vgl::RawImage* _currentMapKd;
  vgl::RawImage* _currentMapKs;
  vgl::RawImage* _currentMapD;

  FramesPerSecond _fps;

  GLuint _shaderWithMaterial;
  GLuint _shaderNoMaterial;
  
  float _currentTime;
  bool _playing;
  int _since;
};


#endif // OBJViewer_renderer_h

