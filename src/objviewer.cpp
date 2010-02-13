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
  mouseX(0),
  mouseY(0),
  mouseButton(0),
  mouseModifiers(0),
  _renderers(),
  _currentRenderer(0),
  _maxTextureWidth(0),
  _maxTextureHeight(0),
  _camera(new Camera())
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(winX, winY);
  glutInitWindowSize(winWidth, winHeight);
  glutCreateWindow("Vil's OBJ Viewer");

  processArgs(argc, argv);

  glutDisplayFunc(doRender);
  glutReshapeFunc(doResize);
  glutKeyboardFunc(doKeyPressed);
  glutMouseFunc(doMousePressed);
  glutMotionFunc(doMouseDragged);
  glutIdleFunc(doRender);
}


OBJViewerApp::~OBJViewerApp()
{
  for (size_t i = 0; i < _renderers.size(); ++i)
    delete _renderers[i];
  delete _camera;
}


void OBJViewerApp::redraw()
{
  currentRenderer()->render(currWidth, currHeight);
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
  if (currentRenderer() != NULL && currentRenderer()->currentModel() != NULL) {
    low = currentRenderer()->currentModel()->low;
    high = currentRenderer()->currentModel()->high;
  }

  switch (key) {
    case 27: // 27 is the ESC key.
      exit(0);
      break;
    case 'f':
      fullscreen = !fullscreen;
      if (fullscreen)
        glutFullScreen();
      else
        glutReshapeWindow(winWidth, winHeight);
      break;
    case 'o':
      currentRenderer()->toggleDrawPolys();
      glutPostRedisplay();
      break;
    case 'p':
      currentRenderer()->toggleDrawPoints();
      glutPostRedisplay();
      break;
    case 'l':
      currentRenderer()->toggleDrawLines();
      glutPostRedisplay();
      break;
    case '0': // Center view.
      currentRenderer()->currentCamera()->centerView(low, high);
      break;
    case '1': // Front view.
      currentRenderer()->currentCamera()->frontView(low, high);
      break;
    case '2': // Back view.
      currentRenderer()->currentCamera()->backView(low, high);
      break;
    case '3': // Left view.
      currentRenderer()->currentCamera()->leftView(low, high);
      break;
    case '4': // Right view.
      currentRenderer()->currentCamera()->rightView(low, high);
      break;
    case '5': // Top view.
      currentRenderer()->currentCamera()->topView(low, high);
      break;
    case '6': // Bottom view.
      currentRenderer()->currentCamera()->bottomView(low, high);
      break;
    case 'c':
      currentRenderer()->currentCamera()->printCameraInfo();
      break;
    case 'g':
      currentRenderer()->printGLInfo();
      break;
    case 'h':
      currentRenderer()->toggleHeadlightType();
      break;
    case ',': case '<':
      _currentRenderer = (_currentRenderer > 0) ? _currentRenderer - 1 : _renderers.size() - 1;
      break;
    case '.': case '>':
      _currentRenderer = (_currentRenderer + 1) % _renderers.size();
      break;
    case '?':
      printf("Esc   Exit the program.\n");
      printf("f     Toggle fullscreen mode on/off.\n");
      printf("o     Toggle display of polyons.\n");
      printf("p     Toggle display of points.\n");
      printf("l     Toggle display of lines.\n");
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
      fprintf(stderr, "key %d pressed\n", key);
      break;
  }
}


void OBJViewerApp::mousePressed(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    mouseX = x;
    mouseY = y;
    mouseButton = button;

    Camera* camera = currentRenderer()->currentCamera();
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

  Camera* camera = currentRenderer()->currentCamera();

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
"Usage: %s [options] <objfile> [ <objfile> ... ]\n"
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

  for (int arg = 0; arg < argc; ++arg) {
    Renderer* renderer = new Renderer(_camera, _maxTextureWidth, _maxTextureHeight);
    const char* modelPath = argv[arg];
    try {
      fprintf(stderr, "Loading model %s\n", modelPath);
      loadModel(renderer, modelPath);
      _renderers.push_back(renderer);
      fprintf(stderr, "Finished loading model %s\n", modelPath);
    } catch (ParseException& e) {
      fprintf(stderr, "%s\n", e.what());
      fprintf(stderr, "Unable to load model. Continuing with default model.\n");
    }
  }

  if (_renderers.size() == 0)
    _renderers.push_back(new Renderer(_camera, _maxTextureWidth, _maxTextureHeight));
}


Renderer* OBJViewerApp::currentRenderer()
{
  return _renderers[_currentRenderer];
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

