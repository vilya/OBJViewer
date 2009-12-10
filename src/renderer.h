#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <map>

#include "image.h"
#include "math3d.h"
#include "model.h"


//
// TYPES
//

enum RenderStyle {
  kLines, kPolygons
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

  Float4 getPosition() const;
  Float4 getUp() const;
  Float4 getTarget() const;
  float getFieldOfViewY() const;
  float getNearClip() const;
  float getFarClip() const;

  void apply(int width, int height);

  void frontView(Model* model, unsigned int frameNum = 0);

private:
  Float4 _position;
  Float4 _up;
  Float4 _target;
  float _fieldOfViewY;
  float _nearClip;
  float _farClip;
};


class Renderer {
public:
  Renderer(Model* model);
  ~Renderer();

  Camera* currentCamera();

  void setStyle(RenderStyle style);

  void render(int width, int height);

private:
  void drawModel(Model* theModel, unsigned int frameNum,
      unsigned int displayList, RenderStyle style);
  void drawDefaultModel(RenderStyle style);
  void setupMaterial(Material* material);
  void setupTexture(GLenum texUnit, Image* texture, Image*& currentTexture);
  void renderFacesForMaterial(Model* model, unsigned int frameNum,
      Material* material, RenderStyle style);

  void loadTexturesForModel(Model* model);
  void loadTexture(Image* tex);
  void drawFPSCounter(int width, int height, float fps);
  void drawBitmapString(float x, float y, void* font, char* str);

private:
  RenderStyle _style;
  Model* _model;
  Camera* _camera;

  Image* _currentMapKa;
  Image* _currentMapKd;
  Image* _currentMapKs;

  FramesPerSecond _fps;
};


#endif // OBJViewer_renderer_h

