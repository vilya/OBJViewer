#ifndef OBJViewer_objviewer_h
#define OBJViewer_objviewer_h

#include "vgl.h"

#include "model.h"
#include "renderer.h"
#include "resources.h"


//
// CLASSES
//

class OBJViewer : public vgl::Viewer {
public:
  // Extra actions.
  enum {
    ACTION_FIRST = 100,

    ACTION_TOGGLE_DRAW_POLYS,
    ACTION_TOGGLE_DRAW_POINTS,
    ACTION_TOGGLE_DRAW_LINES,

    ACTION_VIEW_CENTER,
    ACTION_VIEW_FRONT,
    ACTION_VIEW_BACK,
    ACTION_VIEW_LEFT,
    ACTION_VIEW_RIGHT,
    ACTION_VIEW_TOP,
    ACTION_VIEW_BOTTOM,

    ACTION_FLIP_NORMALS,
    ACTION_TOGGLE_HEADLIGHT_TYPE,

    ACTION_FRAME_PREVIOUS,
    ACTION_FRAME_NEXT,
    ACTION_FRAME_FIRST,
    ACTION_FRAME_LAST,
    ACTION_TOGGLE_PLAYBACK,

    ACTION_PRINT_CAMERA_INFO,
    ACTION_PRINT_GL_INFO,
    ACTION_PRINT_HELP
  };

public:
  OBJViewer(vgl::Camera* camera, Renderer* renderer);

protected:
  virtual int actionForKeyPress(unsigned char key, int x, int y);
  virtual void actionHandler(int action);
};


class ModelBuilder : public vgl::ParserCallbacks
{
public:
  ModelBuilder(Model& model);

  virtual void beginModel(const char* path);
  virtual void endModel();

  virtual void beginFace();
  virtual void endFace();

  virtual void beginVertex();
  virtual void endVertex();

  virtual void beginMaterial(const char* name);
  virtual void endMaterial();

  virtual void indexAttributeParsed(int attr, size_t value);
  virtual void floatAttributeParsed(int attr, float value);
  virtual void vec3fAttributeParsed(int attr, const vgl::Vec3f& value);
  virtual void textureAttributeParsed(int attr, const char* path);
  virtual void stringAttributeParsed(int attr, const char* value);

protected:
  virtual vgl::RawImage* loadTexture(const char* path);

private:
  Model& _model;
  Face* _currentFace;
  Vertex _currentVertex;
  std::string _currentMaterialName;
  Material* _currentMaterial;
  std::map<std::string, vgl::RawImage*> _textures;
};


#endif // OBJViewer_objviewer_h

