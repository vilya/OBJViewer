#ifdef linux
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <getopt.h>
#include <libgen.h>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "objviewer.h"


//
// GLOBAL VARIABLES
//

OBJViewerApp *app = NULL;


//
// FUNCTION DECLARATIONS
//

void doRender();
void doResize(int width, int height);
void doKeyPressed(unsigned char key, int mouseX, int mouseY);
void doMousePressed(int button, int state, int x, int y);
void doMouseDragged(int x, int y);


//
// OBJViewerApp METHODS
//

OBJViewerApp::OBJViewerApp(int argc, char **argv) :
  winX(100), winY(100), winWidth(800), winHeight(600),
  fullscreen(false),
  mouseX(0), mouseY(0),
  _renderer(NULL),
  _model(NULL)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(winX, winY);
  glutInitWindowSize(winWidth, winHeight);
  glutCreateWindow("Vil's OBJ Viewer");

  processArgs(argc, argv);
  _renderer = new Renderer(_model);

  glutDisplayFunc(doRender);
  glutReshapeFunc(doResize);
  glutKeyboardFunc(doKeyPressed);
  glutMouseFunc(doMousePressed);
  glutMotionFunc(doMouseDragged);
}


OBJViewerApp::~OBJViewerApp()
{
  delete _renderer;
  delete _model;
}


void OBJViewerApp::redraw()
{
  _renderer->render(currWidth, currHeight);
}


void OBJViewerApp::changeSize(int width, int height)
{
  if (!fullscreen) {
    winWidth = width;
    winHeight = height;
  }
  currWidth = width;
  currHeight = height;
}


void OBJViewerApp::keyPressed(unsigned char key, int x, int y)
{
  switch (key) {
    case 27: // 27 is the ESC key.
      exit(0);
      break;
    case 'r':
      redraw();
      break;
    case 'f':
      fullscreen = !fullscreen;
      if (fullscreen)
        glutFullScreen();
      else
        glutReshapeWindow(winWidth, winHeight);
      break;
    case 'p':
      _renderer->setStyle(kPolygons);
      glutPostRedisplay();
      break;
    case 'l':
      _renderer->setStyle(kLines);
      glutPostRedisplay();
    default:
      printf("Key pressed: '%c' (0x%x)\n", key, key);
      break;
    }
}


void OBJViewerApp::mousePressed(int button, int state, int x, int y) {
  mouseX = x;
  mouseY = y;
  mouseModifiers = glutGetModifiers();
}


void OBJViewerApp::mouseDragged(int x, int y) {
  int dx = x - mouseX;
  int dy = y - mouseY;

  bool shiftPressed = (mouseModifiers & GLUT_ACTIVE_SHIFT) != 0;
  //bool ctrlPressed = (mouseModifiers & GLUT_ACTIVE_CTRL) != 0;
  //bool altPressed = (mouseModifiers & GLUT_ACTIVE_ALT) != 0;

  if (shiftPressed) {
    if (abs(dx) >= abs(dy))
      _renderer->moveCameraBy(0, 0, dx / 2.0f);
    else
      _renderer->moveCameraBy(0, 0, dy / 2.0f);
  } else {
    _renderer->moveCameraBy(x - mouseX, y - mouseY, 0);
  }
  mouseY = y;
  mouseX = x;
  glutPostRedisplay();
}


void OBJViewerApp::run()
{
  glutMainLoop();
}


void OBJViewerApp::usage(char *progname)
{
    fprintf(stderr,
"Usage: %s [options] <objfile>\n"
"\n"
"Where [options] can be any combination of:\n"
"  -v,--verbose                 Print out some extra information.\n"
"  -h,--help                    Print this message and exit.\n"
            , basename(progname));
}


void OBJViewerApp::processArgs(int argc, char **argv)
{
  const char *short_opts = "h";
  struct option long_opts[] = {
    { "help",               no_argument,        NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  int ch;
  while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
    switch (ch) {
    case 'h':
      usage(argv[0]);
      exit(0);
      break;
    default:
      usage(argv[0]);
      exit(1);
      break;
    }
  }
  argc -= optind;
  argv += optind;
  if (argc > 0) {
    try {
      _model = loadModel(argv[0]);
    } catch (ParseException& e) {
      fprintf(stderr, "Unable to load model. Continuing with default model.\n");
    }
  }
}


void doRender()
{
  app->redraw();
} 


void doResize(int width, int height)
{
  app->changeSize(width, height);
}


void doKeyPressed(unsigned char key, int mouseX, int mouseY)
{
  app->keyPressed(key, mouseX, mouseY);
}


void doMousePressed(int button, int state, int x, int y)
{
  app->mousePressed(button, state, x, y);
}


void doMouseDragged(int x, int y)
{
  app->mouseDragged(x, y);
}


int main(int argc, char **argv)
{
  app = new OBJViewerApp(argc, argv);
  app->run();
  delete app;
  return 0;
}

