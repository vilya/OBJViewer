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

const size_t MAX_FACES_PER_VBO = 200000;


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
// Camera METHODS
//

Camera::Camera() :
  _target(0, 0, 0),
  _rotation(0, 0, 0, 10),
  _fieldOfViewY(30)
{
}


void Camera::moveBy(float x, float y, float z)
{
  _target.x += x;
  _target.y += y;
  _target.z += z;
}


void Camera::zoomBy(float amount)
{
  _rotation.w *= amount;
}


void Camera::rotateByU(float angle)
{
  _rotation.x += angle;
}


void Camera::rotateByV(float angle)
{
  _rotation.y += angle;
}


Float4 Camera::getTarget() const
{
  return _target;
}


float Camera::getDistance() const
{
  return _rotation.w;
}


void Camera::setup(int width, int height, const Float4& low, const Float4& high)
{
  // Here we use the model's bounding sphere to calculate good values for
  // the near and far clipping planes.
  
  // Radius of bounding sphere == half distance between opposite bbox corners
  // FIXME: For some reason the radius ends up being too small, so instead I'm using the diameter for now.
  float radius = length(high - low); 
  // v is the vector from our target to the bbox center, but we'll only need
  // the z component (see below) so that's all we calculate.
  float vz = ((high.z + low.z) / 2.0) - _target.z;
  // Since our direction vector d for the camera is 0,0,1 the dot product of v
  // and d is simply v.z. This gives us the distance along our direction vector
  // at which we're level with the bbox center: _rotation.w + vz. Subtract the
  // radius from that and we've got out near clip plane.
  float nearClip = _rotation.w + vz - radius;
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
  gluPerspective(_fieldOfViewY, double(width) / double(height), nearClip, farClip);
}


void Camera::transformTo()
{
  glTranslatef(-_target.x, -_target.y, -_target.z - _rotation.w);
  glRotatef(_rotation.x, 1, 0, 0);
  glRotatef(_rotation.y, 0, 1, 0);
  glRotatef(_rotation.z, 0, 0, 1);
}


void Camera::centerView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 10.0;
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::frontView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::backView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.z - low.z) / 2 +
      distanceFrom(high.x, low.x, high.y, low.y);
  _rotation = Float4(0, 180, 0, distance);
}


void Camera::leftView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = Float4(0, 270, 0, distance);
}


void Camera::rightView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.x - low.x) / 2 +
      distanceFrom(high.z, low.z, high.y, low.y);
  _rotation = Float4(0, 90, 0, distance);
}


void Camera::topView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = Float4(90, 0, 0, distance);
}


void Camera::bottomView(const Float4& low, const Float4& high)
{
  _target = (high + low) / 2;
  float distance = (high.y - low.y) +
      distanceFrom(high.x, low.x, high.z, low.z);
  _rotation = Float4(270, 0, 0, distance);
}


float Camera::distanceFrom(float highU, float lowU, float highV, float lowV) const
{
  float opposite = fmaxf(highU - lowU, highV - lowV) / 2.0;
  float angle = (_fieldOfViewY / 2.0) * M_PI / 180.0;
  float adjacent = (opposite / tanf(angle));
  return adjacent;
}


void Camera::printCameraInfo() const
{
  fprintf(stderr, "Camera at:\n");
  fprintf(stderr, "target = { %f, %f, %f }\n", _target.x, _target.y, _target.z);
  fprintf(stderr, "rotation = { %f, %f, %f, %f }\n",
                  _rotation.x, _rotation.y, _rotation.z, _rotation.w);
}


//
// RenderGroup METHODS
//

RenderGroup::RenderGroup(Material* iMaterial, RenderGroupType iType) :
  _material(iMaterial),
  _type(iType),
  _size(0),
  _hasTexCoords(false),
  _hasNormalCoords(false),
  _coords(),
  _coordsID(0),
  _indexes(),
  _indexesID(0)
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
    (3 + (_hasTexCoords ? 2 : 0) + (_hasNormalCoords ? 4 : 0) + (_hasColors ? 3 : 0));
  glVertexPointer(3, GL_FLOAT, stride, 0);

  if (_material != NULL) {
    if (_hasTexCoords) {
      RawImage* textures[] =
          { _material->mapKa, _material->mapKd, _material->mapKs, _material->mapD };
      for (int i = 0; i < 4; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glClientActiveTexture(GL_TEXTURE0 + i);
        if (textures[i] != NULL) {
          glEnable(GL_TEXTURE_2D);
          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
          glBindTexture(GL_TEXTURE_2D, textures[i]->getTexID());
          glTexCoordPointer(2, GL_FLOAT, stride, (const GLvoid*)(sizeof(float)*3));
        } else {
          glDisable(GL_TEXTURE_2D);
          glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
      }
    } else {
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
  } else {
    for (int i = 0; i < 4; ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      glClientActiveTexture(GL_TEXTURE0 + i);
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
  }

  if (_hasNormalCoords) {
    glEnableClientState(GL_NORMAL_ARRAY);
    GLuint offset = sizeof(float) * (_hasTexCoords ? 5 : 3);
    glNormalPointer(GL_FLOAT, stride, (const GLvoid*)offset);
  } else {
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  if (_hasColors) {
    glEnableClientState(GL_COLOR_ARRAY);
    GLuint offset = sizeof(float) * ((_hasTexCoords ? 5 : 3) + (_hasNormalCoords ? 4 : 0));
    glColorPointer(3, GL_FLOAT, stride, (const GLvoid*)offset);
  } else if (_material != NULL) {
    glDisableClientState(GL_COLOR_ARRAY);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _material->Ka.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _material->Kd.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _material->Ks.data);
  } else {
    glDisableClientState(GL_COLOR_ARRAY);
    float defaultColor[] = { 0.4, 0.4, 0.4, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultColor);
  }

  switch (_type) {
    case kTriangleGroup:
      glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
      break;
    case kQuadGroup:
      glDrawElements(GL_QUADS, _size, GL_UNSIGNED_INT, 0);
      break;
    case kPolygonGroup:
      glDrawElements(GL_POLYGON, _size, GL_UNSIGNED_INT, 0);
      break;
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  if (_material != NULL)
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (_hasNormalCoords)
    glDisableClientState(GL_NORMAL_ARRAY);
  if (_hasColors)
    glDisableClientState(GL_COLOR_ARRAY);

  // Release the VBOs.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//
// Renderer METHODS
//

Renderer::Renderer(size_t maxTextureWidth, size_t maxTextureHeight) :
  _style(kPolygons),
  _headlightType(kDirectional),
  _drawLights(false),
  _model(NULL),
  _camera(new Camera()),
  _maxTextureWidth(maxTextureWidth),
  _maxTextureHeight(maxTextureHeight),
  _renderGroups(),
  _currentMapKa(NULL),
  _currentMapKd(NULL),
  _currentMapKs(NULL),
  _currentMapD(NULL),
  _fps()
{
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glShadeModel(GL_SMOOTH);
}


Renderer::~Renderer()
{
  delete _camera;
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


void Renderer::setStyle(RenderStyle style)
{
  _style = style;
}


void Renderer::toggleDrawLights()
{
  _drawLights = !_drawLights;
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
    _camera->setup(width, height, _model->low, _model->high);
  } else {
    Float4 low(-1, -1, -1, 1);
    Float4 high(1, 1, 1, 1);
    _camera->setup(width, height, low, high);
  }

  // Draw the scene.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Put a light at the same position as the camera.
  headlight(GL_LIGHT0, Float4(1, 1, 1, 1));

  _camera->transformTo();
  if (_model != NULL) {
    std::list<RenderGroup*>::iterator iter;
    for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
      RenderGroup* group = *iter;
      group->render();
   }
  } else {
    drawDefaultModel(_style);
  }
  drawFPSCounter(width, height, _fps.fps());

  glutSwapBuffers();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  _fps.increment();
}


void Renderer::prepare()
{
  prepareRenderGroups();
  prepareMaterials();
  //prepareShaders();
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
      (*groupMap)[material].push_back(new RenderGroup(material, type));
    }

    std::list<RenderGroup*>& groups = (*groupMap)[material];
    if (!isTriangle || groups.front()->size() >= MAX_FACES_PER_VBO)
      groups.push_front(new RenderGroup(material, type));
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

  // Get the shader source code.
  const char* vertexShaderSrc = loadShader("vertex.sl");
  const char* fragmentShaderSrc = loadShader("fragment.sl");
  if (vertexShaderSrc == NULL || fragmentShaderSrc == NULL)
    return;

  // Associate the source code strings with the shader handles. We can delete
  // the strings after these calls.
  glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
  glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);

  // Compile the shaders.
  glCompileShader(vertexShader);
  glCompileShader(fragmentShader);

  // Create a program object and attach the shaders to it.
  GLuint programObject = glCreateProgram();
  glAttachShader(programObject, fragmentShader);
  glAttachShader(programObject, vertexShader);

  // Link the program.
  glLinkProgram(programObject);
  
  // Enable the program.
  glUseProgram(programObject);
}


void Renderer::drawDefaultModel(RenderStyle style)
{
  switch (style) {
  case kLines:
    glutWireTeapot(1.0f);
    break;
  case kPolygons:
  default:
    glutSolidTeapot(1.0f);
    break;
  }
}


void Renderer::setupMaterial(Material* material)
{
  setupTexture(GL_TEXTURE0, material->mapKa, _currentMapKa);
  setupTexture(GL_TEXTURE1, material->mapKd, _currentMapKd);
  setupTexture(GL_TEXTURE2, material->mapKs, _currentMapKs);
  setupTexture(GL_TEXTURE3, material->mapD, _currentMapD);
}


void Renderer::setupTexture(GLenum texUnit, RawImage* texture, RawImage*& currentTexture)
{
  if (texture != currentTexture) {
    glActiveTexture(texUnit);
    if (texture != NULL) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture->getTexID());
      checkGLError("Error setting up texture");
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    currentTexture = texture;
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
  if (isMatte) {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  } else {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
  }

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

  if (_drawLights) {
    glBegin(GL_LINES);
    glVertex3f(pos.x - 10.5, pos.y, pos.z);
    glVertex3f(pos.x + 10.5, pos.y, pos.z);

    glVertex3f(pos.x, pos.y - 10.5, pos.z);
    glVertex3f(pos.x, pos.y + 10.5, pos.z);
  
    glVertex3f(pos.x, pos.y, pos.z - 10.5);
    glVertex3f(pos.x, pos.y, pos.z + 10.5);
    glEnd();
  }
  glPopMatrix();
}


void Renderer::drawFPSCounter(int width, int height, float fps)
{
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(1, width - 1, 1, height - 1, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor4f(1.0f, 0.0f, 0.0f, 1.0);

  char buf[20];
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "%5.2f fps", fps);

  drawBitmapString(10, 10, GLUT_BITMAP_8_BY_13, buf);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glEnable(GL_LIGHTING);
}


void Renderer::drawBitmapString(float x, float y, void* font, char* str)
{
  float xPos = x;
  for (char* ch = str; *ch != '\0'; ++ch) {
    glRasterPos2f(xPos, y);
    glutBitmapCharacter(font, *ch);
    xPos += glutBitmapWidth(font, *ch);
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


void checkGLError(const char *errMsg, const char *okMsg)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "%s: %s (%d)\n", errMsg, gluErrorString(err), err);
  } else if (okMsg != NULL) {
    fprintf(stderr, "%s\n", okMsg);
  }
}

