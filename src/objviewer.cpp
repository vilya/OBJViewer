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
  _modelPath(NULL),
  _maxTextureWidth(0), _maxTextureHeight(0)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(winX, winY);
  glutInitWindowSize(winWidth, winHeight);
  glutCreateWindow("Vil's OBJ Viewer");

  processArgs(argc, argv);
  _renderer = new Renderer(_maxTextureWidth, _maxTextureHeight);
  try {
    loadModel(_renderer, _modelPath);
    fprintf(stderr, "Finished loading model %s\n", _modelPath);
  } catch (ParseException& e) {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "Unable to load model. Continuing with default model.\n");
  }

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
  Float4 low(-1, -1, -1, 1);
  Float4 high(1, 1, 1, 1);
  if (_renderer != NULL && _renderer->currentModel() != NULL) {
    low = _renderer->currentModel()->low;
    high = _renderer->currentModel()->high;
  }

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
    case '0': // Center view.
      _renderer->currentCamera()->centerView(low, high);
      break;
    case '1': // Front view.
      _renderer->currentCamera()->frontView(low, high);
      break;
    case '2': // Back view.
      _renderer->currentCamera()->backView(low, high);
      break;
    case '3': // Left view.
      _renderer->currentCamera()->leftView(low, high);
      break;
    case '4': // Right view.
      _renderer->currentCamera()->rightView(low, high);
      break;
    case '5': // Top view.
      _renderer->currentCamera()->topView(low, high);
      break;
    case '6': // Bottom view.
      _renderer->currentCamera()->bottomView(low, high);
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
      printf("0     Center view (in the middle of the object).\n");
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
    // This bit doesn't work on OS X - no mouse wheel events are received (at
    // least by this method).
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
    camera->moveBy(-dx * camera->getDistance() / winWidth,
                   dy * camera->getDistance() / winHeight, 0);
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
"Usage: %s [options] <objfile>\n"
"\n"
"Where [options] can be any combination of:\n"
"  -t,--max-texture-size WxH    Specify the maximum texture size. Any textures\n"
"                               larger than this will be downsampled. The\n"
"                               default is no maximum.\n"
"  -h,--help                    Print this message and exit.\n"
"\n"
"You can also press keys to perform various functions while viewing a model.\n"
"To see a list of these, press the '?' key."
            , basename(progname));
}


bool OBJViewerApp::parseDimensions(char* dimensions, size_t& width, size_t& height)
{
  char* tok = strtok(dimensions, "x");
  if (tok == NULL)
    return false;
  width = atoi(tok);

  tok = strtok(dimensions, "x");
  if (tok == NULL)
    height = width;
  else
    height = atoi(tok);
  
  return true;
}


void OBJViewerApp::processArgs(int argc, char **argv)
{
  const char *short_opts = "ht:";
  struct option long_opts[] = {
    { "max-texture-size",   required_argument,  NULL, 't' },
    { "help",               no_argument,        NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  int ch;
  while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
    switch (ch) {
    case 't':
      if (!parseDimensions(optarg, _maxTextureWidth, _maxTextureHeight)) {
        usage(argv[0]);
        exit(0);
      }
      break;
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
  if (argc > 0)
    _modelPath = argv[0];
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

