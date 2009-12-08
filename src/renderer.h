#ifndef OBJViewer_renderer_h
#define OBJViewer_renderer_h

#include <map>
#include "image.h"
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


class Renderer {
public:
  Renderer(Model* model);
  ~Renderer();

  void moveCameraBy(float u, float v, float distance);
  void moveCameraTo(float u, float v, float distance);

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
  Model *_model;
  float _camU, _camV, _camDist;

  Image* _currentMapKa;
  Image* _currentMapKd;
  Image* _currentMapKs;

  FramesPerSecond _fps;
};


#endif // OBJViewer_renderer_h
