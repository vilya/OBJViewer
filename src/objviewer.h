#ifndef OBJViewer_objviewer_h
#define OBJViewer_objviewer_h

#include "model.h"
#include "renderer.h"


//
// CLASSES
//

class OBJViewerApp : public ParserCallbacks {
public:
  OBJViewerApp(int argc, char **argv);
  ~OBJViewerApp();

  void redraw();
  void changeSize(int width, int height);
  void keyPressed(unsigned char key, int x, int y);
  void mousePressed(int button, int state, int x, int y);
  void mouseDragged(int x, int y);
  void run();

  // Parser callbacks
  virtual void beginModel(const char* path);
  virtual void endModel();
  virtual void coordParsed(const Float4& coord);
  virtual void texCoordParsed(const Float4& coord);
  virtual void normalParsed(const Float4& normal);
  virtual void colorParsed(const Float4& color);
  virtual void faceParsed(Face* face);
  virtual void materialParsed(const std::string& name, Material* material);
  virtual void textureParsed(RawImage* texture);

private:
  //! Prints help about the command line syntax and options to stderr.
  void usage(char *progname);

  bool parseDimensions(char* dimensions, size_t& width, size_t& height);

  //! Process the command line arguments.
  void processArgs(int argc, char **argv);

  Renderer* currentRenderer();

private:
  int winX, winY, winWidth, winHeight, currWidth, currHeight;
  bool fullscreen;
  int mouseX, mouseY, mouseButton, mouseModifiers;
  Model* _model;
  Renderer* _renderer;
  size_t _maxTextureWidth, _maxTextureHeight;

  Camera* _camera;
};


#endif // OBJViewer_objviewer_h

