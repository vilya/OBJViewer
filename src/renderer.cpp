#define GL_GLEXT_PROTOTYPES 1

#ifdef linux
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#include <cmath>
#include <cstdlib>

#include "renderer.h"


//
// CONSTANTS
//

const size_t MAX_FACES_PER_VBO = 1000000;


//
// FUNCTION DECLARATIONS
//

void checkGLError(const char *errMsg, const char *okMsg = NULL);


//
// FramesPerSecond METHODS
//

FramesPerSecond::FramesPerSecond() : _framesDrawn(0), _since(0), _fps(0.0)
{
}


void FramesPerSecond::increment()
{
  ++_framesDrawn;

  int now = glutGet(GLUT_ELAPSED_TIME);
  if (now - _since > 1000) {
    _fps = _framesDrawn * 1000.0 / (now - _since);
    _framesDrawn = 0;
    _since = now;
  }
}


float FramesPerSecond::fps() const
{
  return _fps;
}


//
// RenderGroup METHODS
//

RenderGroup::RenderGroup(Material* iMaterial, RenderGroupType iType, GLuint defaultTextureID) :
  _material(iMaterial),
  _type(iType),
  _size(0),
  _hasTexCoords(false),
  _hasNormalCoords(false),
  _coords(),
  _coordsID(0),
  _indexes(),
  _indexesID(0),
  _defaultTextureID(defaultTextureID)
{
}


Material* RenderGroup::getMaterial() const
{
  return _material;
}


void RenderGroup::add(Model* model, Face* face)
{
  if (_size == 0) {
    _hasTexCoords = (face->material != NULL) && (*face)[0].vt >= 0;
    _hasNormalCoords = (*face)[0].vn >= 0;
    _hasColors = (*face)[0].c >= 0;
  }

  for (size_t i = 0; i < face->size(); ++i) {
    int vi = (*face)[i].v;
    Float4& v = model->v[vi];
    _coords.push_back(v.x);
    _coords.push_back(v.y);
    _coords.push_back(v.z);

    if (_hasTexCoords) {
      int vti = (*face)[i].vt;
      Float4& vt = model->vt[vti];
      _coords.push_back(vt.x);
      _coords.push_back(vt.y);
    } else {
      _coords.push_back(0.5);
      _coords.push_back(0.5);
    }

    if (_hasNormalCoords) {
      int vni = (*face)[i].vn;
      Float4& vn = model->vn[vni];
      _coords.push_back(vn.x);
      _coords.push_back(vn.y);
      _coords.push_back(vn.z);
      _coords.push_back(vn.w);
    }

    if (_hasColors) {
      int ci = (*face)[i].c;
      Float4& c = model->colors[ci];
      _coords.push_back(c.r);
      _coords.push_back(c.g);
      _coords.push_back(c.b);
    }

    _indexes.push_back(_indexes.size());
  }
  _size = _indexes.size();
}


size_t RenderGroup::size() const
{
  return _size;
}


void RenderGroup::prepare()
{
  // Get a VBO ID for the coords, upload the coords to the VBO, then delete the local copy.
  glGenBuffers(1, &_coordsID);
  glBindBuffer(GL_ARRAY_BUFFER, _coordsID);
  glBufferData(GL_ARRAY_BUFFER,
      sizeof(float) * _coords.size(), &_coords[0], GL_STATIC_DRAW);
  _coords.clear();

  // Do the same for the indexes.
  glGenBuffers(1, &_indexesID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      sizeof(unsigned int) * _indexes.size(), &_indexes[0], GL_STATIC_DRAW);
  _indexes.clear();
}


void RenderGroup::render()
{
  glBindBuffer(GL_ARRAY_BUFFER, _coordsID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);

  glEnableClientState(GL_VERTEX_ARRAY);
  GLuint stride = sizeof(float) *
    (5 + (_hasNormalCoords ? 4 : 0) + (_hasColors ? 3 : 0));
  glVertexPointer(3, GL_FLOAT, stride, 0);

  RawImage* textures[4] = { NULL, NULL, NULL, NULL };
  if (_material != NULL) {
    textures[0] = _material->mapKa;
    textures[1] = _material->mapKd;
    textures[2] = _material->mapKs;
    textures[3] = _material->mapD;
  }
  for (int i = 0; i < 4; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glClientActiveTexture(GL_TEXTURE0 + i);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (textures[i] != NULL)
      glBindTexture(GL_TEXTURE_2D, textures[i]->getTexID());
    else
      glBindTexture(GL_TEXTURE_2D, _defaultTextureID);
    glTexCoordPointer(2, GL_FLOAT, stride, (const GLvoid*)(sizeof(float)*3));
  }

  if (_hasNormalCoords) {
    glEnableClientState(GL_NORMAL_ARRAY);
    GLuint offset = sizeof(float) * 5;
    glNormalPointer(GL_FLOAT, stride, (const GLvoid*)offset);
  } else {
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  if (_hasColors) {
    glEnableClientState(GL_COLOR_ARRAY);
    GLuint offset = sizeof(float) * (5 + (_hasNormalCoords ? 4 : 0));
    glColorPointer(3, GL_FLOAT, stride, (const GLvoid*)offset);
  } else if (_material != NULL) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _material->Ka.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _material->Kd.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _material->Ks.data);
  } else {
    float defaultColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultColor);
  }

  if (_material != NULL) {
    float shininess = std::min(_material->Ns * 128.0, 128.0);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  } else {
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);
  }

  switch (_type) {
    case kTriangleGroup:
      glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
      break;
    case kPolygonGroup:
      glDrawElements(GL_POLYGON, _size, GL_UNSIGNED_INT, 0);
      break;
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (_hasNormalCoords)
    glDisableClientState(GL_NORMAL_ARRAY);
  if (_hasColors)
    glDisableClientState(GL_COLOR_ARRAY);

  // Release the VBOs.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void RenderGroup::renderPoints()
{
  glBindBuffer(GL_ARRAY_BUFFER, _coordsID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glDisable(GL_LIGHTING);

  glEnableClientState(GL_VERTEX_ARRAY);
  GLuint stride = sizeof(float) *
    (5 + (_hasNormalCoords ? 4 : 0) + (_hasColors ? 3 : 0));
  glVertexPointer(3, GL_FLOAT, stride, 0);
  
  glColor3f(0.0, 1.0, 1.0);
  glEnable(GL_POLYGON_OFFSET_POINT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  glPolygonOffset(0, -5);
  glPointSize(5);

  switch (_type) {
    case kTriangleGroup:
      glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
      break;
    case kPolygonGroup:
      glDrawElements(GL_POLYGON, _size, GL_UNSIGNED_INT, 0);
      break;
  }

  glPolygonOffset(0, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_POLYGON_OFFSET_POINT);
  glDisableClientState(GL_VERTEX_ARRAY);
  glEnable(GL_LIGHTING);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void RenderGroup::renderLines()
{
  glBindBuffer(GL_ARRAY_BUFFER, _coordsID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glDisable(GL_LIGHTING);

  glEnableClientState(GL_VERTEX_ARRAY);
  GLuint stride = sizeof(float) *
    (5 + (_hasNormalCoords ? 4 : 0) + (_hasColors ? 3 : 0));
  glVertexPointer(3, GL_FLOAT, stride, 0);
  
  glColor3f(0.8, 0.8, 0.8);
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonOffset(0, -3);

  switch (_type) {
    case kTriangleGroup:
      glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
      break;
    case kPolygonGroup:
      glDrawElements(GL_POLYGON, _size, GL_UNSIGNED_INT, 0);
      break;
  }

  glPolygonOffset(0, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glDisableClientState(GL_VERTEX_ARRAY);
  glEnable(GL_LIGHTING);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


//
// Renderer METHODS
//

Renderer::Renderer(Camera* camera, size_t maxTextureWidth, size_t maxTextureHeight) :
  _headlightType(kDirectional),
  _drawPolys(true),
  _drawPoints(false),
  _drawLines(false),
  _model(NULL),
  _camera(camera),
  _maxTextureWidth(maxTextureWidth),
  _maxTextureHeight(maxTextureHeight),
  _renderGroups(),
  _currentMapKa(NULL),
  _currentMapKd(NULL),
  _currentMapKs(NULL),
  _currentMapD(NULL),
  _fps(),
  _defaultTexture(NULL),
  _programObject(0)
{
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (unsigned int i = 0; i < 4; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glEnable(GL_TEXTURE_2D);
  }

  //float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glShadeModel(GL_SMOOTH);

  // Create a default texture (1x1 pixel, pure white).
  _defaultTexture = new RawImage(GL_RGBA, 4, 4, 4);
  unsigned char* pixels = _defaultTexture->getPixels();
  for (size_t i = 0; i < 4 * 4 * 4; ++i)
    pixels[i] = 255;
}


Renderer::~Renderer()
{
  delete _model;
  std::list<RenderGroup*>::iterator iter;
  for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter)
    delete *iter;
}


Camera* Renderer::currentCamera()
{
  return _camera;
}


Model* Renderer::currentModel()
{
  return _model;
}


void Renderer::toggleDrawPolys()
{
  _drawPolys = !_drawPolys;
}


void Renderer::toggleDrawPoints()
{
  _drawPoints = !_drawPoints;
}


void Renderer::toggleDrawLines()
{
  _drawLines = !_drawLines;
}


void Renderer::toggleHeadlightType()
{
  if (_headlightType == kSpotlight)
    _headlightType = kDirectional;
  else
    _headlightType = kSpotlight;
}


void Renderer::printGLInfo()
{
  fprintf(stderr, "GL_MAX_COMBINED_TEXTURE_UNITS = %d\n",
    glGet(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
  fprintf(stderr, "GL_MAX_TEXTURE_COORDS = %d\n",
    glGet(GL_MAX_TEXTURE_COORDS));
}


void Renderer::render(int width, int height)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Apply the camera settings.
  if (_model != NULL) {
    setupCamera(width, height, _model->low, _model->high);
  } else {
    Float4 low(-1, -1, -1, 1);
    Float4 high(1, 1, 1, 1);
    setupCamera(width, height, low, high);
  }

  // Draw the scene.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Put a light at the same position as the camera.
  headlight(GL_LIGHT0, Float4(1, 1, 1, 1));

  transformToCamera();
  if (_model != NULL) {
    std::list<RenderGroup*>::iterator iter;

    if (_drawPolys) {
      for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
        RenderGroup* group = *iter;
        group->render();
      }
    }

    if (_drawPoints) {
      glUseProgram(0);
      glDisable(GL_LIGHTING);
      for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
        RenderGroup* group = *iter;
        group->renderPoints();
      }
      glEnable(GL_LIGHTING);
      glUseProgram(_programObject);
    }

    if (_drawLines) {
      glUseProgram(0);
      glDisable(GL_LIGHTING);
      for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
        RenderGroup* group = *iter;
        group->renderLines();
      }
      glEnable(GL_LIGHTING);
      glUseProgram(_programObject);
    }
  } else {
    drawDefaultModel();
  }
  drawHUD(width, height, _fps.fps());

  glutSwapBuffers();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  _fps.increment();
}


void Renderer::setupCamera(int width, int height, const Float4& low, const Float4& high)
{
  Float4 target = _camera->getTarget();
  float distance = _camera->getDistance();

  // Here we use the model's bounding sphere to calculate good values for
  // the near and far clipping planes.
  
  // Radius of bounding sphere == half distance between opposite bbox corners
  // FIXME: For some reason the radius ends up being too small, so instead I'm
  // using the diameter for now.
  float radius = length(high - low); 
  // v is the vector from our target to the bbox center, but we'll only need
  // the z component (see below) so that's all we calculate.
  float vz = ((high.z + low.z) / 2.0) - target.z;
  // Since our direction vector d for the camera is 0,0,1 the dot product of v
  // and d is simply v.z. This gives us the distance along our direction vector
  // at which we're level with the bbox center: _rotation.w + vz. Subtract the
  // radius from that and we've got out near clip plane.
  float nearClip = distance + vz - radius;
  // The far clip plane will always be the near clip plane plus the diameter.
  float farClip = nearClip + 2 * radius;
  // Make sure the near clip plane doesn't end up behind us.
  if (nearClip < 0.01)
    nearClip = 0.01;
  // Make sure the far clip plane doesn't end up behind the near clip plane.
  if (farClip <= nearClip)
    farClip = nearClip + 0.01;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  glLoadIdentity();
  glViewport(0, 0, width, height); // Set the viewport to be the entire window
  gluPerspective(_camera->getFieldOfViewY(), double(width) / double(height),
      nearClip, farClip);
}


void Renderer::transformToCamera()
{
  Float4 target = _camera->getTarget();
  Float4 rotation = _camera->getRotation();
  float distance = _camera->getDistance();
  glTranslatef(-target.x, -target.y, -target.z - distance);
  glRotatef(rotation.x, 1, 0, 0);
  glRotatef(rotation.y, 0, 1, 0);
  glRotatef(rotation.z, 0, 0, 1);
}


void Renderer::prepare()
{
  loadTexture(_defaultTexture, false);
  prepareModel();
  prepareRenderGroups();
  prepareMaterials();
  prepareShaders();
}


void Renderer::prepareModel()
{
  if (_model->vn.size() == 0) {
    fprintf(stderr, "Calculating normals...\n");
    _model->vn.reserve(_model->v.size());
    while (_model->vn.size() < _model->v.size())
      _model->vn.push_back(Float4());

    for (size_t i = 0; i < _model->faces.size(); ++i) {
      Face& face = *_model->faces[i];
      Float4& a = _model->v[face[0].v];
      Float4& b = _model->v[face[1].v];
      Float4& c = _model->v[face[2].v];
      Float4 faceNormal = normalize(cross(b - a, c - a));

      for (size_t j = 0; j < face.size(); ++j) {
        _model->vn[face[j].v] = _model->vn[face[j].v] + faceNormal;
        face[j].vn = face[j].v;
      }
    }

    for (size_t i = 0; i < _model->vn.size(); ++i)
      _model->vn[i] = normalize(_model->vn[i]);
  }
}


void Renderer::prepareRenderGroups()
{
  // Create the render groups. Each material will have up to one group for
  // each of triangles, quads and general polygons.
  fprintf(stderr, "Creating render groups...\n");
  std::map<Material*, std::list<RenderGroup*> > triangles;
  std::map<Material*, std::list<RenderGroup*> > polys;
  std::map<Material*, std::list<RenderGroup*> > transparentTriangles;
  std::map<Material*, std::list<RenderGroup*> > transparentPolys;

  for (size_t i = 0; i < _model->faces.size(); ++i) {
    Face* face = _model->faces[i];
    if (face->size() < 3)
      continue;

    // Note that we've already triangulated quads by this point.
    Material* material = face->material;

    bool isTransparent = (material != NULL) && (material->d != 1 || material->mapD != NULL);
    bool isTriangle = (face->size() == 3);

    RenderGroupType type = isTriangle ? kTriangleGroup : kPolygonGroup;
    std::map<Material*, std::list<RenderGroup*> >* groupMap;

    if (isTriangle)
      groupMap = isTransparent ? &transparentTriangles : &triangles;
    else
      groupMap = isTransparent ? &transparentPolys : &polys;

    if (groupMap->find(material) == groupMap->end()) {
      (*groupMap)[material] = std::list<RenderGroup*>();
      (*groupMap)[material].push_back(new RenderGroup(material, type, _defaultTexture->getTexID()));
    }

    std::list<RenderGroup*>& groups = (*groupMap)[material];
    if ((!isTriangle && groups.front()->size() > 0) || groups.front()->size() >= MAX_FACES_PER_VBO)
      groups.push_front(new RenderGroup(material, type, _defaultTexture->getTexID()));
    groups.front()->add(_model, face);
  }

  // Copy the render groups into the _renderGroups list. Make sure the groups
  // with transparent materials go last, so that they'll be rendered after the
  // opaque materials.
  fprintf(stderr, "Sorting render groups...\n");
  std::map<Material*, std::list<RenderGroup*> >::iterator listIter;
  std::list<RenderGroup*>::iterator iter;

  for (listIter = triangles.begin(); listIter != triangles.end(); ++listIter) {
    for (iter = listIter->second.begin(); iter != listIter->second.end(); ++iter)
      _renderGroups.push_back(*iter);
  }
  for (listIter = polys.begin(); listIter != polys.end(); ++listIter) {
    for (iter = listIter->second.begin(); iter != listIter->second.end(); ++iter)
      _renderGroups.push_back(*iter);
  }
  _transparentGroupsStart = _renderGroups.size();
  for (listIter = transparentTriangles.begin(); listIter != transparentTriangles.end(); ++listIter) {
    for (iter = listIter->second.begin(); iter != listIter->second.end(); ++iter)
      _renderGroups.push_back(*iter);
  }
  for (listIter = transparentPolys.begin(); listIter != transparentPolys.end(); ++listIter) {
    for (iter = listIter->second.begin(); iter != listIter->second.end(); ++iter)
      _renderGroups.push_back(*iter);
  }

  // Prepare the render groups.
  fprintf(stderr, "Preparing render groups...\n");
  std::list<RenderGroup*>::iterator groupIter;
  for (groupIter = _renderGroups.begin(); groupIter != _renderGroups.end(); ++groupIter)
    (*groupIter)->prepare();
}


void Renderer::prepareMaterials()
{
  // Prepare the materials.
  fprintf(stderr, "Preparing materials...\n");
  std::map<std::string, Material*>::iterator m;
  for (m = _model->materials.begin(); m != _model->materials.end(); ++m) {
    Material* material = m->second;
    if (material->d != 1 || material->mapD != NULL) {
      material->Ka.a = material->d;
      material->Kd.a = material->d;
      material->Ks.a = material->d;
    }
  }
}


void Renderer::prepareShaders()
{
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  GLint status;

  // Get the shader source code.
  const char* vertexShaderSrc = loadShader("vertex.vert");
  const char* fragmentShaderSrc = loadShader("fragment.frag");
  if (vertexShaderSrc == NULL || fragmentShaderSrc == NULL)
    return;

  // Associate the source code strings with the shader handles. We can delete
  // the strings after these calls.
  glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
  glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);

  // Compile the shaders.
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    fprintf(stderr, "Vertex shader failed to compile:\n");
    printShaderInfoLog(vertexShader);
  }

  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    fprintf(stderr, "Fragment shader failed to compile.\n");
    printShaderInfoLog(fragmentShader);
  }

  // Create a program object and attach the shaders to it.
  _programObject = glCreateProgram();
  glAttachShader(_programObject, fragmentShader);
  glAttachShader(_programObject, vertexShader);

  // Link the program.
  glLinkProgram(_programObject);
  glGetProgramiv(_programObject, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    fprintf(stderr, "Shader program failed to link.\n");
    printProgramInfoLog(_programObject);
  }

  // Enable the program.
  glUseProgram(_programObject);

  // Pass the textures to the program.
  const char* names[] = { "mapKa", "mapKd", "mapKs", "mapD" };
  for (int i = 0; i < 4; ++i) {
    GLint location = glGetUniformLocation(_programObject, names[i]);
    glUniform1i(location, i);
  }
}


void Renderer::drawDefaultModel()
{
  if (_drawPolys) {
    glutSolidTeapot(1.0f);
  } else {
    glutWireTeapot(1.0f);
  }
}


void Renderer::loadTextures(std::list<RenderGroup*>& groups)
{
  Material* currentMaterial = NULL;

  std::list<RenderGroup*>::iterator iter;
  for (iter = groups.begin(); iter != groups.end(); ++iter) {
    Material* material = (*iter)->getMaterial();
    if (material != NULL && material != currentMaterial) {
      loadTexture(material->mapD, true);
      loadTexture(material->mapKa, false);
      loadTexture(material->mapKd, false);
      loadTexture(material->mapKs, false);
    }
  }
}


void Renderer::loadTexture(RawImage* tex, bool isMatte)
{
  // If tex is null, or tex is already loaded.
  if (tex == NULL || tex->getTexID() != (unsigned int)-1)
    return;

  GLenum targetType;
  if (isMatte) {
    targetType = GL_ALPHA;
  } else {
    switch (tex->getType()) {
    case GL_BGR:
      targetType = GL_RGB;
      break;
    case GL_BGRA:
      targetType = GL_RGBA;
      break;
    default:
      targetType = tex->getType();
      break;
    }
  }
  checkGLError("Some GL error before loading texture.");

  GLuint texID;
  glGenTextures(1, &texID);

  glBindTexture(GL_TEXTURE_2D, texID);
  checkGLError("Texture not bound.");
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  checkGLError("Pixel storage format not set.");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  checkGLError("Texture parameters not set.");
  
  unsigned int downsampleX = 1;
  unsigned int downsampleY = 1;
  if (_maxTextureWidth > 0) {
    while (tex->getWidth() / downsampleX > _maxTextureWidth)
      ++downsampleX;
  }
  if (_maxTextureHeight > 0) {
    while (tex->getHeight() / downsampleY > _maxTextureHeight)
      ++downsampleY;
  }

  if (downsampleX == 1 || downsampleY == 1) {
    glTexImage2D(GL_TEXTURE_2D, 0, targetType, tex->getWidth(), tex->getHeight(), 0,
                 tex->getType(), GL_UNSIGNED_BYTE, tex->getPixels());
  } else {
    fprintf(stderr, "Downsampling texture %d: %ux%u --> %ux%u\n", texID,
      tex->getWidth(), tex->getHeight(),
      tex->getWidth() / downsampleX, tex->getHeight() / downsampleY);
    RawImage* downsampledTex = downsample(tex, downsampleX, downsampleY);
    glTexImage2D(GL_TEXTURE_2D, 0, targetType,
                 downsampledTex->getWidth(), downsampledTex->getHeight(), 0,
                 downsampledTex->getType(), GL_UNSIGNED_BYTE, downsampledTex->getPixels());
    delete downsampledTex;
  }

  checkGLError("Texture failed to load.");

  tex->setTexID(texID);
  tex->deletePixels();
}


void Renderer::headlight(GLenum light, const Float4& color)
{
  Float4 pos(0, 0, 0, 1); // Relative to camera position.
  Float4 direction(0, 0, -1, 1);

  glPushMatrix();
  glLoadIdentity();

  //glLighti(light, GL_SPOT_EXPONENT, 30);
  if (_headlightType == kSpotlight)
    glLighti(light, GL_SPOT_CUTOFF, 15);
  else
    glLighti(light, GL_SPOT_CUTOFF, 180);
  glLightfv(light, GL_SPOT_DIRECTION, direction.data);
  glLightfv(light, GL_POSITION, pos.data);
  glLightfv(light, GL_DIFFUSE, color.data);

  glPopMatrix();
}


void Renderer::drawHUD(int width, int height, float fps)
{
  glUseProgram(0);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(1, width - 1, 1, height - 1, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  float defaultColor[] = { 0.0, 1.0, 1.0, 1.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultColor);

  char buf[4096];
  if (_model != NULL) {
    sprintf(buf,
        "%5.2f FPS\n"
        "%lu faces\n"
        "%lu vertices\n"
        "%lu materials\n"
        "%lu render groups",
        fps, _model->faces.size(), _model->v.size(), _model->materials.size(), _renderGroups.size());
    drawBitmapString(10, 70, GLUT_BITMAP_8_BY_13, buf);
  } else {
    sprintf(buf,
        "%5.2f FPS\n"
        "Anyone for tea?",
        fps);
    drawBitmapString(10, 25, GLUT_BITMAP_8_BY_13, buf);
  }
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glUseProgram(_programObject);
}


void Renderer::drawBitmapString(float x, float y, void* font, char* str)
{
  float xPos = x;
  for (char* ch = str; *ch != '\0'; ++ch) {
    glRasterPos2f(xPos, y);
    switch (*ch) {
      case '\n':
        xPos = x;
        y -= 15;
        break;
      default:
        glutBitmapCharacter(font, *ch);
        xPos += glutBitmapWidth(font, *ch);
        break;
    }
  }
}


const char* Renderer::loadShader(const char* path)
{
  FILE* shaderFile = fopen(path, "r");
  if (shaderFile == NULL)
    return NULL;

  // Get the length of the file.
  fseek(shaderFile, 0, SEEK_END);
  size_t length = ftell(shaderFile);
  fseek(shaderFile, 0, SEEK_SET);

  char* text = new char[length + 1];
  fread(text, sizeof(const char), length, shaderFile);
  text[length] = '\0';

  fclose(shaderFile);
  return text;
}

void Renderer::beginModel(const char* path)
{
  // Clear out any old model data.
  delete _model;
  for (std::list<RenderGroup*>::iterator i = _renderGroups.begin(); i != _renderGroups.end(); ++i)
    delete *i;
  _renderGroups.clear();
  _transparentGroupsStart = (size_t)-1;
  _currentMapKa = NULL;
  _currentMapKd = NULL;
  _currentMapKs = NULL;
  _currentMapD = NULL;

  // Start a new model.
  _model = new Model();
}


void Renderer::endModel()
{
  prepare();
  loadTextures(_renderGroups);
  _camera->frontView(_model->low, _model->high);
  checkGLError("Error during preparation.");
}


void Renderer::coordParsed(const Float4& coord)
{
  _model->addV(coord);
}


void Renderer::texCoordParsed(const Float4& coord)
{
  _model->vt.push_back(coord);
}


void Renderer::normalParsed(const Float4& normal)
{
  _model->vn.push_back(normal);
}


void Renderer::colorParsed(const Float4& color)
{
  _model->colors.push_back(color);
}


void Renderer::faceParsed(Face* face)
{
  if (face->size() == 4) {
    Face* newFace = new Face(face->material);
    newFace->vertexes.push_back(face->vertexes[0]);
    newFace->vertexes.push_back(face->vertexes[1]);
    newFace->vertexes.push_back(face->vertexes[2]);
    _model->faces.push_back(newFace);

    newFace = new Face(face->material);
    newFace->vertexes.push_back(face->vertexes[0]);
    newFace->vertexes.push_back(face->vertexes[2]);
    newFace->vertexes.push_back(face->vertexes[3]);
    _model->faces.push_back(newFace);

    delete face;
  } else {
    _model->faces.push_back(face);
  }
}


void Renderer::materialParsed(const std::string& name, Material* material)
{
  _model->materials[name] = material;
}


void Renderer::textureParsed(RawImage* texture)
{
  // At the moment we don't need to do anything here, but we probably will do soon...
}


GLint Renderer::glGet(GLenum what)
{
  GLint val;
  glGetIntegerv(what, &val);
  return val;
}


void Renderer::printShaderInfoLog(GLuint obj)
{
  int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0) {
	  infoLog = new char[infologLength];
	  glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "%s\n", infoLog);
    delete[] infoLog;
	}
}

void Renderer::printProgramInfoLog(GLuint obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
  if (infologLength > 0) {
    infoLog = new char[infologLength];
    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "%s\n", infoLog);
    delete[] infoLog;
  }
}


void checkGLError(const char *errMsg, const char *okMsg)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "%s: %s (%d)\n", errMsg, gluErrorString(err), err);
  } else if (okMsg != NULL) {
    fprintf(stderr, "%s\n", okMsg);
  }
}

