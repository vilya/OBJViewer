#include "objviewer.h"

#include "curve.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <libgen.h>


//
// OBJViewer METHODS
//

OBJViewer::OBJViewer(Renderer* renderer) :
  vgl::Viewer("Vil's OBJ Viewer", 1024, 768, renderer)
{
}


int OBJViewer::actionForKeyPress(unsigned char key, int x, int y)
{
  switch (key) {
    case 'o': return ACTION_TOGGLE_DRAW_POLYS;
    case 'p': return ACTION_TOGGLE_DRAW_POINTS;
    case 'l': return ACTION_TOGGLE_DRAW_LINES;
    case '0': return ACTION_VIEW_CENTER;
    case '1': return ACTION_VIEW_FRONT;
    case '2': return ACTION_VIEW_BACK;
    case '3': return ACTION_VIEW_LEFT;
    case '4': return ACTION_VIEW_RIGHT;
    case '5': return ACTION_VIEW_TOP;
    case '6': return ACTION_VIEW_BOTTOM;
    case 'n': return ACTION_FLIP_NORMALS;
    case 'h': return ACTION_TOGGLE_HEADLIGHT_TYPE;
    case ',': return ACTION_FRAME_PREVIOUS;
    case '.': return ACTION_FRAME_NEXT;
    case 'm': return ACTION_FRAME_FIRST;
    case '/': return ACTION_FRAME_LAST;
    case ' ': return ACTION_TOGGLE_PLAYBACK;
    case 'c': return ACTION_PRINT_CAMERA_INFO;
    case 'g': return ACTION_PRINT_GL_INFO;
    case '?': return ACTION_PRINT_HELP;
    default: return vgl::Viewer::actionForKeyPress(key, x, y);
  }
}


void OBJViewer::actionHandler(int action)
{
  Renderer* renderer = dynamic_cast<Renderer*>(_renderer);

  vgl::Vec3f low(-1, -1, -1);
  vgl::Vec3f high(1, 1, 1);
  if (renderer != NULL && renderer->currentModel() != NULL) {
    low = renderer->currentModel()->low;
    high = renderer->currentModel()->high;
  }

  switch (action) {
    case ACTION_TOGGLE_DRAW_POLYS: renderer->toggleDrawPolys(); break;
    case ACTION_TOGGLE_DRAW_POINTS: renderer->toggleDrawPoints(); break;
    case ACTION_TOGGLE_DRAW_LINES: renderer->toggleDrawLines(); break;
    /*
    case ACTION_VIEW_CENTER: _camera->centerView(low, high); break;
    case ACTION_VIEW_FRONT: _camera->frontView(low, high); break;
    case ACTION_VIEW_BACK: _camera->backView(low, high); break;
    case ACTION_VIEW_LEFT: _camera->leftView(low, high); break;
    case ACTION_VIEW_RIGHT: _camera->rightView(low, high); break;
    case ACTION_VIEW_TOP: _camera->topView(low, high); break;
    case ACTION_VIEW_BOTTOM: _camera->bottomView(low, high); break;
    */
    case ACTION_FLIP_NORMALS: renderer->flipNormals(); break;
    case ACTION_TOGGLE_HEADLIGHT_TYPE: renderer->toggleHeadlightType(); break;
    case ACTION_FRAME_PREVIOUS: renderer->previousFrame(); break;
    case ACTION_FRAME_NEXT: renderer->nextFrame(); break;
    case ACTION_FRAME_FIRST: renderer->firstFrame(); break;
    case ACTION_FRAME_LAST: renderer->lastFrame(); break;
    case ACTION_TOGGLE_PLAYBACK: renderer->togglePlaying(); break;
    /*
    case ACTION_PRINT_CAMERA_INFO: _camera->printCameraInfo(); break;
    */
    case ACTION_PRINT_GL_INFO: renderer->printGLInfo(); break;
    case ACTION_PRINT_HELP:
      printf("Esc   Exit the program.\n");
      printf("f     Toggle fullscreen mode on/off.\n");
      printf("o     Toggle display of polyons.\n");
      printf("p     Toggle display of points.\n");
      printf("l     Toggle display of lines.\n");
      /*
      printf("0     Center view (in the middle of the object).\n");
      printf("1     Front view.\n");
      printf("2     Back view.\n");
      printf("3     Left view.\n");
      printf("4     Right view.\n");
      printf("5     Top view.\n");
      printf("6     Bottom view.\n");
      printf("c     Print current camera info.\n");
      */
      printf("g     Print OpenGL info.\n");
      printf("n     Flip the normals.\n");
      printf("h     Toggle the type of headlight between directional and spot.\n");
      printf("m     Jump back to the first frame.\n");
      printf(",     Step back 1 frame.\n");
      printf(".     Step forward 1 frame.\n");
      printf("/     Jump forward to the last frame.\n");
      printf("Space Play/pause animation.\n");
      printf("\n");
      break;
    default:
      vgl::Viewer::actionHandler(action);
      break;
  }
  glutPostRedisplay();
}


//
// ModelBuilder METHODS
//


ModelBuilder::ModelBuilder(Model& model) :
  _model(model),
  _currentFace(NULL),
  _currentVertex(-1, -1, -1, -1),
  _currentMaterial(new Material())
{
}


void ModelBuilder::beginModel(const char* path)
{
  _model.newKeyframe();
}


void ModelBuilder::endModel()
{
}


void ModelBuilder::beginFace()
{
  _currentFace = new Face(_currentMaterial);
}


void ModelBuilder::endFace()
{
  if (_currentFace == NULL || _model.numKeyframes() > 1)
    return;

  // Special case, for performace: triangulate quad faces.
  if (_currentFace->size() == 4) {
    Face* newFace = new Face(_currentFace->material);
    newFace->vertexes.push_back(_currentFace->vertexes[0]);
    newFace->vertexes.push_back(_currentFace->vertexes[1]);
    newFace->vertexes.push_back(_currentFace->vertexes[2]);
    _model.addFace(newFace);

    newFace = new Face(_currentFace->material);
    newFace->vertexes.push_back(_currentFace->vertexes[0]);
    newFace->vertexes.push_back(_currentFace->vertexes[2]);
    newFace->vertexes.push_back(_currentFace->vertexes[3]);
    _model.addFace(newFace);

    delete _currentFace;
  } else {
    _model.addFace(_currentFace);
  }

  _currentFace = NULL;
}


void ModelBuilder::beginVertex()
{
  _currentVertex = Vertex(-1, -1, -1, -1);
}


void ModelBuilder::endVertex()
{
  if (_currentFace != NULL)
    _currentFace->vertexes.push_back(_currentVertex);
}


void ModelBuilder::beginMaterial(const char* name)
{
  _currentMaterial = new Material();
  _model.addMaterial(name, _currentMaterial);
}


void ModelBuilder::endMaterial()
{
}


void ModelBuilder::indexAttributeParsed(int attr, size_t value)
{
  switch (attr) {
    case kCoordRef: _currentVertex.v = (int)value; break;
    case kTexCoordRef: _currentVertex.vt = (int)value; break;
    case kNormalRef: _currentVertex.vn = (int)value; break;
    default: break;
  }
}


void ModelBuilder::floatAttributeParsed(int attr, float value)
{
  switch (attr) {
    case kDissolve: _currentMaterial->d = value; break;
    case kSpecularExponent: _currentMaterial->Ns = value; break;
    default: break;
  }
}


void ModelBuilder::vec3fAttributeParsed(int attr, const vgl::Vec3f& value)
{
  // FIXME: Not handling per-vertex colours anymore. :-(
  switch (attr) {
    case kAmbientColor: _currentMaterial->Ka = vgl::Vec4f(value.r, value.g, value.b, 1.0); break;
    case kDiffuseColor: _currentMaterial->Kd = vgl::Vec4f(value.r, value.g, value.b, 1.0); break;
    case kSpecularColor: _currentMaterial->Ks = vgl::Vec4f(value.r, value.g, value.b, 1.0); break;
    case kTransmissivity: _currentMaterial->Tf = vgl::Vec4f(value.r, value.g, value.b, 1.0); break;

    case kCoord: _model.addV(value); break;
    case kTexCoord: _model.addVt(vgl::Vec2f(value.x, value.y)); break;
    case kNormal: _model.addVn(value); break;

    default: break;
  }
}


void ModelBuilder::textureAttributeParsed(int attr, const char* path)
{
  switch (attr) {
    case kAmbientColor: _currentMaterial->mapKa = loadTexture(path); break;
    case kDiffuseColor: _currentMaterial->mapKd = loadTexture(path); break;
    case kSpecularColor: _currentMaterial->mapKs = loadTexture(path); break;
    case kDissolve: _currentMaterial->mapD = loadTexture(path); break;
    case kBumpMap: _currentMaterial->mapBump = loadTexture(path); break;
    default: break;
  }
}


void ModelBuilder::stringAttributeParsed(int attr, const char* value)
{
  switch (attr) {
    case kMaterialName:
      _currentMaterial = _model.materials[std::string(value)];
      break;
    default:
      break;
  }
}


vgl::RawImage* ModelBuilder::loadTexture(const char* path)
{
  std::map<std::string, vgl::RawImage*>::iterator texIter = _textures.find(path);
  if (texIter != _textures.end())
    return texIter->second;

  fprintf(stderr, "Loading texture %s...", path);

  vgl::RawImage* tex = NULL;
  try {
    tex = new vgl::RawImage(path);
    _textures[std::string(path)] = tex;
  } catch (vgl::ImageException& ex) {
    throw vgl::ParseException("Error loading texture map: %s", ex.what());
  }

  if (tex != NULL)
    fprintf(stderr, " %dx%d pixels.\n", tex->getWidth(), tex->getHeight());
  return tex;
}


//
// Functions
//

static void usage(char *progname)
{
    fprintf(stderr,
"Usage: %s [options] <objfile> [ <objfile> ... ]\n"
"\n"
"Where [options] can be any combination of:\n"
"  -t,--max-texture-size WxH    Specify the maximum texture size. Any textures\n"
"                               larger than this will be downsampled. The\n"
"                               default is no maximum.\n"
"  -f,--fps FPS                 The target playback speed for the animation.\n"
"                               If our actual frame rate is different to this\n"
"                               the frames will be interpolated. The default\n"
"                               is 30.0 fps.\n"
"  -h,--help                    Print this message and exit.\n"
"\n"
"You can also press keys to perform various functions while viewing a model.\n"
"To see a list of these, press the '?' key.\n"
            , basename(progname));
}


static bool parseDimensions(char* dimensions, size_t& width, size_t& height)
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


int main(int argc, char **argv)
{
  ResourceManager resources;
  resources.textures.addAppDir(argv[0], "..");
  resources.shaders.addAppDir(argv[0], "..");

  Model model;

  size_t maxTextureWidth = 0;
  size_t maxTextureHeight = 0;
  float animFPS = 30.0;

  const char *short_opts = "ht:f:";
  struct option long_opts[] = {
    { "max-texture-size",   required_argument,  NULL, 't' },
    { "fps",                required_argument,  NULL, 'f' },
    { "help",               no_argument,        NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  int ch;
  while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
    switch (ch) {
    case 't':
      if (!parseDimensions(optarg, maxTextureWidth, maxTextureHeight)) {
        usage(argv[0]);
        exit(0);
      }
      break;
    case 'f':
      animFPS = atof(optarg);
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

  ModelBuilder builder(model);
  for (int arg = 0; arg < argc; ++arg) {
    const char* modelPath = argv[arg];
    try {
      fprintf(stderr, "Loading model %s\n", modelPath);
      vgl::loadModel(&builder, modelPath);
      fprintf(stderr, "Finished loading model %s\n", modelPath);
    } catch (vgl::ParseException& e) {
      fprintf(stderr, "%s\n", e.what());
      fprintf(stderr, "Unable to load model. Continuing with default model.\n");
    }
  }

  Renderer renderer(&resources, &model, maxTextureWidth, maxTextureHeight, animFPS);

  OBJViewer app(&renderer);
  app.run();
  return 0;
}

