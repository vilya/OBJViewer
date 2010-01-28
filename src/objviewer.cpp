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
#include "parser.h"


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
  mouseX(0), mouseY(0), mouseButton(0), mouseModifiers(0),
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
  glutIdleFunc(doRender);
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
      break;
    case 'i':
      _renderer->toggleDrawLights();
      glutPostRedisplay();
      break;
    case '1': // Front view.
      _renderer->currentCamera()->frontView(_model, 0);
      break;
    case '2': // Back view.
      _renderer->currentCamera()->backView(_model, 0);
      break;
    case '3': // Left view.
      _renderer->currentCamera()->leftView(_model, 0);
      break;
    case '4': // Right view.
      _renderer->currentCamera()->rightView(_model, 0);
      break;
    case '5': // Top view.
      _renderer->currentCamera()->topView(_model, 0);
      break;
    case '6': // Bottom view.
      _renderer->currentCamera()->bottomView(_model, 0);
      break;
    case 'c':
      _renderer->currentCamera()->printCameraInfo();
      break;
    case 'g':
      _renderer->printGLInfo();
      break;
    case 'h':
      _renderer->toggleHeadlightType();
      break;
    case '?':
      printf("Esc   Exit the program.\n");
      printf("r     Force a redraw.\n");
      printf("f     Toggle fullscreen mode on/off.\n");
      printf("p     Display model as polygons.\n");
      printf("l     Display model as lines.\n");
      printf("i     Draw markers to show the positions of lights.\n");
      printf("1     Front view.\n");
      printf("2     Back view.\n");
      printf("3     Left view.\n");
      printf("4     Right view.\n");
      printf("5     Top view.\n");
      printf("6     Bottom view.\n");
      printf("c     Print current camera info.\n");
      printf("g     Print OpenGL info.\n");
      printf("h     Toggle the type of headlight between directional and spot.\n");
      printf("\n");
      break;
    default:
      break;
  }
}


void OBJViewerApp::mousePressed(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    mouseX = x;
    mouseY = y;
    mouseButton = button;

    Camera* camera = _renderer->currentCamera();
    if (mouseButton == 3) // Mouse wheel up
      camera->zoomBy(1.0 / 1.1);
    else if (mouseButton == 4)  // Mouse wheel down
      camera->zoomBy(1.1);
  }
}


void OBJViewerApp::mouseDragged(int x, int y) {
  int dx = x - mouseX;
  int dy = y - mouseY;

  Camera* camera = _renderer->currentCamera();
  switch (mouseButton) {
  case GLUT_LEFT_BUTTON:
    camera->rotateByU(dy);
    camera->rotateByV(dx);
    break;
  case GLUT_MIDDLE_BUTTON:
    if (abs(dx) >= abs(dy))
      camera->zoomBy(powf(1.1, dx / 2.0f));
    else
      camera->zoomBy(powf(1.1, dy / 2.0f));
    break;
  case GLUT_RIGHT_BUTTON:
    camera->moveBy(dx, dy, 0);
    break;
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
"Usage: %s [options] <objfile> <startFrame> <endFrame>\n"
"\n"
"Where [options] can be any combination of:\n"
"  -v,--verbose                 Print out some extra information.\n"
"  -h,--help                    Print this message and exit.\n"
            , basename(progname));
}


void OBJViewerApp::processArgs(int argc, char **argv)
{
  unsigned int startFrame = 0;
  unsigned int endFrame = 0;

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
    if (argc > 1) {
      startFrame = (unsigned int)atoi(argv[1]);
      endFrame = startFrame;
    }
    if (argc > 2)
      endFrame = (unsigned int)atoi(argv[2]);

    try {
      _model = loadModel(argv[0], startFrame, endFrame);
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

