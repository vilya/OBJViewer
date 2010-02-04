#ifndef OBJViewer_objviewer_h
#define OBJViewer_objviewer_h

#include "model.h"
#include "renderer.h"


//
// CLASSES
//

class OBJViewerApp {
public:
  OBJViewerApp(int argc, char **argv);
  ~OBJViewerApp();

  void redraw();
  void changeSize(int width, int height);
  void keyPressed(unsigned char key, int x, int y);
  void mousePressed(int button, int state, int x, int y);
  void mouseDragged(int x, int y);
  void run();

private:
  //! Prints help about the command line syntax and options to stderr.
  void usage(char *progname);

  bool parseDimensions(char* dimensions, size_t& width, size_t& height);

  //! Process the command line arguments.
  void processArgs(int argc, char **argv);

private:
  int winX, winY, winWidth, winHeight, currWidth, currHeight;
  bool fullscreen;
  int mouseX, mouseY, mouseButton, mouseModifiers;
  Renderer* _renderer;
  const char* _modelPath;
  size_t _maxTextureWidth, _maxTextureHeight;
};


#endif // OBJViewer_objviewer_h

