#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <list>
#include <map>

#include <imagelib.h>
#include "math3d.h"
#include "model.h"


//
// TYPES
//

enum RenderStyle {
  kLines, kPolygons
};

enum LightType {
  kDirectional, kSpotlight
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


struct RenderGroup {
  Material* mat;
  int firstID;
  int lastID;

  RenderGroup(Material* material, int firstID, int lastID);
};


class Renderer {
public:
  Renderer(Model* model);
  ~Renderer();

  Camera* currentCamera();

  void setStyle(RenderStyle style);
  void toggleDrawLights();
  void toggleHeadlightType();
  void printGLInfo();

  void render(int width, int height);

private:
  void prepare(std::list<RenderGroup>& groups);

  void drawModel(Model* theModel, RenderStyle style, std::list<RenderGroup>& groups);
  void drawDefaultModel(RenderStyle style);
  void setupMaterial(Material* material);
  void setupTexture(GLenum texUnit, RawImage* texture, RawImage*& currentTexture);
  void renderGroup(Model* model, RenderStyle style, const RenderGroup& group);

  void loadTexturesForModel(Model* model);
  void loadTexture(RawImage* tex);
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
  std::list<RenderGroup> _renderGroupsPolys;
  std::list<RenderGroup> _renderGroupsLines;

  RawImage* _currentMapKa;
  RawImage* _currentMapKd;
  RawImage* _currentMapKs;
  RawImage* _currentMapD;

  FramesPerSecond _fps;
};


#endif // OBJViewer_renderer_h

