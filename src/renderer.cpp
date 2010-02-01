#ifdef linux
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#endif

#include <cmath>
#include <cstdlib>

#include "renderer.h"


//
// CONSTANTS
//

const size_t MAX_FACES_PER_DISPLAYLIST = 20000;


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
  for (size_t i = 0; i < face->size(); ++i) {
    int vi = (*face)[i].v;
    int vti = (*face)[i].vt;
    int vni = (*face)[i].vn;

    // XXX: what to do if the vertex doesn't have tex coords or a normal?

    Float4& v = model->v[vi];
    Float4& vt = model->vt[vti];
    Float4& vn = model->vn[vni];
  
    _coords.push_back(v.x);
    _coords.push_back(v.y);
    _coords.push_back(v.z);
    _coords.push_back(v.w);

    _coords.push_back(vt.x);
    _coords.push_back(vt.y);

    _coords.push_back(vn.x);
    _coords.push_back(vn.y);
    _coords.push_back(vn.z);

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
  glGenBuffersARB(1, &_coordsID);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, _coordsID);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB,
      sizeof(float) * _coords.size(), &_coords[0], GL_STATIC_DRAW_ARB);
  _coords.clear();

  // Do the same for the indexes.
  glGenBuffersARB(1, &_indexesID);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _indexesID);
  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
      sizeof(unsigned int) * _indexes.size(), &_indexes[0], GL_STATIC_DRAW_ARB);
  _indexes.clear();
}


void RenderGroup::render()
{
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, _coordsID);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _indexesID);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(4, GL_FLOAT, sizeof(float)*9, 0);
  if (_material != NULL) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, _material->Ka.data);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, _material->Kd.data);
    glMaterialfv(GL_FRONT, GL_SPECULAR, _material->Ks.data);

    RawImage* textures[] = { _material->mapD, _material->mapKa, _material->mapKd, _material->mapKs };
    for (int i = 0; i < 4; ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      glClientActiveTexture(GL_TEXTURE0 + i);
      if (textures[i] != NULL) {
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindTexture(GL_TEXTURE_2D, textures[i]->getTexID());
        glTexCoordPointer(2, GL_FLOAT, sizeof(float)*9, (const GLvoid*)(sizeof(float)*4));
      } else {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      }
    }
  } else {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    float defaultColor[] = { 1.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, defaultColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, defaultColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, defaultColor);
  }
  glNormalPointer(GL_FLOAT, sizeof(float)*9, (const GLvoid*)(sizeof(float)*6));

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
  glDisableClientState(GL_NORMAL_ARRAY);

  // Release the VBOs.
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


//
// Renderer METHODS
//

Renderer::Renderer(Model* model) :
  _style(kPolygons),
  _headlightType(kDirectional),
  _drawLights(false),
  _model(model),
  _camera(new Camera()),
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

  float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glShadeModel(GL_SMOOTH);

  if (_model != NULL) {
    prepare();
    loadTextures(_renderGroups);
    _camera->frontView(_model->low, _model->high);
  }
  checkGLError("Error during initialisation.");
}


Renderer::~Renderer()
{
  if (_camera != NULL)
    delete _camera;
}


Camera* Renderer::currentCamera()
{
  return _camera;
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


GLint Renderer::glGet(GLenum what)
{
  GLint val;
  glGetIntegerv(what, &val);
  return val;
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
  // Create the render groups. Each material will have up to one group for
  // each of triangles, quads and general polygons.
  std::map<Material*, RenderGroup*> triangles;
  std::map<Material*, RenderGroup*> quads;
  std::map<Material*, std::list<RenderGroup*> > polys;
  std::map<Material*, RenderGroup*> transparentTriangles;
  std::map<Material*, RenderGroup*> transparentQuads;
  std::map<Material*, std::list<RenderGroup*> > transparentPolys;

  for (size_t i = 0; i < _model->faces.size(); ++i) {
    Face* face = _model->faces[i];
    if (face->size() < 3)
      continue;

    Material* material = face->material;
    if (material != NULL && (material->d != 1 || material->mapD != NULL)) {
      switch (face->size()) {
        case 3:
          if (transparentTriangles.find(material) == transparentTriangles.end())
            transparentTriangles[material] = new RenderGroup(material, kTriangleGroup);
          transparentTriangles[material]->add(_model, face);
          break;
        case 4:
          if (transparentQuads.find(material) == transparentQuads.end())
            transparentQuads[material] = new RenderGroup(material, kQuadGroup);
          transparentQuads[material]->add(_model, face);
          break;
        default:
          {
            if (transparentPolys.find(material) == transparentPolys.end())
              transparentPolys[material] = std::list<RenderGroup*>();
            RenderGroup* polyGroup = new RenderGroup(material, kPolygonGroup);
            polyGroup->add(_model, face);
            transparentPolys[material].push_back(polyGroup);
          }
          break;
      }
    } else {
      switch (face->size()) {
        case 3:
          if (triangles.find(material) == triangles.end())
            triangles[material] = new RenderGroup(material, kTriangleGroup);
          triangles[material]->add(_model, face);
          break;
        case 4:
          if (quads.find(material) == quads.end())
            quads[material] = new RenderGroup(material, kQuadGroup);
          quads[material]->add(_model, face);
          break;
        default:
          {
            if (polys.find(material) == polys.end())
              polys[material] = std::list<RenderGroup*>();
            RenderGroup* polyGroup = new RenderGroup(material, kPolygonGroup);
            polyGroup->add(_model, face);
            polys[material].push_back(polyGroup);
          }
          break;
      }
    }
  }

  // Copy the render groups into the _renderGroups list. Make sure the groups
  // with transparent materials go last, so that they'll be rendered after the
  // opaque materials.
  std::map<Material*, RenderGroup*>::iterator iter;
  std::map<Material*, std::list<RenderGroup*> >::iterator listIter;

  for (iter = triangles.begin(); iter != triangles.end(); ++iter)
    _renderGroups.push_back(iter->second);
  for (iter = quads.begin(); iter != quads.end(); ++iter)
    _renderGroups.push_back(iter->second);
  for (listIter = polys.begin(); listIter != polys.end(); ++listIter) {
    std::list<RenderGroup*>::iterator polyIter;
    for (polyIter = listIter->second.begin(); polyIter != listIter->second.end(); ++polyIter)
      _renderGroups.push_back(*polyIter);
  }

  _transparentGroupsStart = _renderGroups.size();

  for (iter = transparentTriangles.begin(); iter != transparentTriangles.end(); ++iter)
    _renderGroups.push_back(iter->second);
  for (iter = transparentQuads.begin(); iter != transparentQuads.end(); ++iter)
    _renderGroups.push_back(iter->second);
  for (listIter = transparentPolys.begin(); listIter != transparentPolys.end(); ++listIter) {
    std::list<RenderGroup*>::iterator polyIter;
    for (polyIter = listIter->second.begin(); polyIter != listIter->second.end(); ++polyIter)
      _renderGroups.push_back(*polyIter);
  }

  // Prepare the render groups.
  std::list<RenderGroup*>::iterator groupIter;
  for (groupIter = _renderGroups.begin(); groupIter != _renderGroups.end(); ++groupIter)
    (*groupIter)->prepare();

  // Prepare the materials.
  std::map<std::string, Material>::iterator m;
  for (m = _model->materials.begin(); m != _model->materials.end(); ++m) {
    Material& material = m->second;
    if (material.d != 1 || material.mapD != NULL) {
      material.Ka.a = material.d;
      material.Kd.a = material.d;
      material.Ks.a = material.d;
    }
  }
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
  setupTexture(GL_TEXTURE0, material->mapD, _currentMapD);
  setupTexture(GL_TEXTURE1, material->mapKa, _currentMapKa);
  setupTexture(GL_TEXTURE2, material->mapKd, _currentMapKd);
  setupTexture(GL_TEXTURE3, material->mapKs, _currentMapKs);
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
      loadTexture(material->mapD);
      loadTexture(material->mapKa);
      loadTexture(material->mapKd);
      loadTexture(material->mapKs);
    }
  }
}


void Renderer::loadTexture(RawImage* tex)
{
  // If tex is null, or tex is already loaded.
  if (tex == NULL || tex->getTexID() != (unsigned int)-1)
    return;

  GLenum targetType;
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
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
  glTexImage2D(GL_TEXTURE_2D, 0, targetType, tex->getWidth(), tex->getHeight(), 0,
               tex->getType(), GL_UNSIGNED_BYTE, tex->getPixels());
  checkGLError("Texture failed to load.");

  tex->setTexID(texID);
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


void checkGLError(const char *errMsg, const char *okMsg)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "%s: %s (%d)\n", errMsg, gluErrorString(err), err);
  } else if (okMsg != NULL) {
    fprintf(stderr, "%s\n", okMsg);
  }
}

