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


class Renderer {
public:
    Renderer(Model* model);
    ~Renderer();

    void moveCameraBy(float u, float v, float distance);
    void moveCameraTo(float u, float v, float distance);

    void setStyle(RenderStyle style);

    void render(int width, int height);

private:
    //! Load all textures for the model onto the GPU.
    void loadTexturesForModel(Model* theModel);

    //! Draw the specified model as either filled polygons or lines.
    void drawModel(Model* theModel, bool filledPolygons);

    //! Load an image file onto the GPU as a texture
    void loadTexture(GLenum texUnit, int texID, Image& tex);

private:
    RenderStyle _style;
    Model *_model;
    float _camU, _camV, _camDist;
};


#endif // OBJViewer_renderer_h
