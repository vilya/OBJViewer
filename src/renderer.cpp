#ifdef linux
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#include <cmath>
#include <cstdlib>

#include "renderer.h"


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

  _position(0, 0, -10),
  _up(0, 1, 0),
  _target(0, 0, 0),
  _fieldOfViewY(30),
  _nearClip(0.1),
  _farClip(5000)
{
}


void Camera::moveBy(float x, float y, float z)
{
  _position.x += x;
  _position.y += y;
  _position.z += z;

  _target.x += x;
  _target.y += y;
  _target.z += z;
}


void Camera::zoomBy(float amount)
{
  _position.x *= amount;
  _position.y *= amount;
  _position.z *= amount;
}


void Camera::rotateByU(float angle)
{
  Float4 direction = _position - _target;
  direction = Matrix4::rotateY(angle) * direction;
  _position = direction + _target;
}


void Camera::rotateByV(float angle)
{
  Float4 direction = _position - _target;
  direction = Matrix4::rotateX(angle) * direction;
  _position = direction + _target;
}


Float4 Camera::getPosition() const
{
  return _position;
}


Float4 Camera::getUp() const
{
  return _up;
}


Float4 Camera::getTarget() const
{
  return _target;
}


float Camera::getFieldOfViewY() const
{
  return _fieldOfViewY;
}


float Camera::getNearClip() const
{
  return _nearClip;
}


float Camera::getFarClip() const
{
  return _farClip;
}


void Camera::apply(int width, int height)
{
  // Position the camera
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glViewport(0, 0, width, height); // Set the viewport to be the entire window

  // Set up the camera position
  gluPerspective(_fieldOfViewY, double(width) / double(height), _nearClip, _farClip);
  gluLookAt(
      _position.x, _position.y, _position.z,
      _target.x, _target.y, _target.z,
      _up.x, _up.y, _up.z
  );
}


void Camera::frontView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 1, 0);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.z = frame.high.z +
      distanceFrom(frame.high.x, frame.low.x, frame.high.y, frame.low.y);
}


void Camera::backView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 1, 0);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.z = frame.low.z -
      distanceFrom(frame.high.x, frame.low.x, frame.high.y, frame.low.y);
}


void Camera::leftView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 1, 0);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.x = frame.low.x -
      distanceFrom(frame.high.z, frame.low.z, frame.high.y, frame.low.y);
}


void Camera::rightView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 1, 0);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.x = frame.high.x +
      distanceFrom(frame.high.z, frame.low.z, frame.high.y, frame.low.y);
}


void Camera::topView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 0, -1);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.y = frame.high.y +
      distanceFrom(frame.high.x, frame.low.x, frame.high.z, frame.low.z);
}


void Camera::bottomView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _up = Float4(0, 0, 1);
  _target = (frame.high + frame.low) / 2;
  _position = _target;

  _position.y = frame.low.y -
      distanceFrom(frame.high.x, frame.low.x, frame.high.z, frame.low.z);
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
  fprintf(stderr, "pos    = { %f, %f, %f }\n", _position.x, _position.y, _position.z);
  fprintf(stderr, "up     = { %f, %f, %f }\n", _up.x, _up.y, _up.z);
  fprintf(stderr, "target = { %f, %f, %f }\n", _target.x, _target.y, _target.z);
}


//
// Renderer METHODS
//

Renderer::Renderer(Model* model) :
  _style(kPolygons),
  _drawLights(false),
  _model(model),
  _camera(new Camera()),
  _currentMapKa(NULL),
  _currentMapKd(NULL),
  _currentMapKs(NULL),
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
    _camera->frontView(_model);
    _model->displayListStart = glGenLists(_model->materials.size() * 2);
    loadTexturesForModel(_model);
    drawModel(_model, 0, kLines);
    drawModel(_model, 0, kPolygons);
  }
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


void Renderer::render(int width, int height)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Apply the camera settings.
  _camera->apply(width, height);

  // Draw the scene.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Put a light at the same position as the camera.
  Float4 light = _camera->getPosition();
  glLightfv(GL_LIGHT0, GL_POSITION, light.data);
  if (_drawLights)
    drawLight(light);

  if (_model != NULL) {
    std::map<std::string, Material>::iterator m;
    unsigned int displayListID = _model->displayListStart +
      _model->materials.size() * (unsigned int)_style;
    for (m = _model->materials.begin(); m != _model->materials.end(); ++m) {
      Material* material = &m->second;
      setupMaterial(material);
      glCallList(displayListID);
      ++displayListID;
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


void Renderer::drawModel(Model* model, unsigned int frameNum, RenderStyle style)
{
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);

  _currentMapKa = NULL;
  _currentMapKd = NULL;
  _currentMapKs = NULL;

  unsigned int displayListID = model->displayListStart + model->materials.size() * (unsigned int)style;

  std::map<std::string, Material>::iterator m;
  for (m = model->materials.begin(); m != model->materials.end(); ++m) {
    Material *material = &m->second;
    setupMaterial(material);
    glNewList(displayListID, GL_COMPILE);
    renderFacesForMaterial(model, frameNum, material, style);
    glEndList();
    ++displayListID;
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
  setupTexture(GL_TEXTURE0, material->mapKa, _currentMapKa);
  setupTexture(GL_TEXTURE1, material->mapKd, _currentMapKd);
  setupTexture(GL_TEXTURE2, material->mapKs, _currentMapKs);
}


void Renderer::setupTexture(GLenum texUnit, RawImage* texture, RawImage*& currentTexture)
{
  if (texture != currentTexture) {
    glActiveTexture(texUnit);
    checkGLError("No luck activating texture");
    if (texture != NULL) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture->getTexID());
      /* Disabled for now because it kills things when you have a couple of large (4k) textures:
      gluBuild2DMipmaps(GL_TEXTURE_2D, texture->getType(),
          texture->getWidth(), texture->getHeight(),
          texture->getType(), GL_UNSIGNED_BYTE, texture->getPixels());
      */
      checkGLError("Error setting up texture");
    } else {
      glDisable(GL_TEXTURE_2D);
      checkGLError("Error disabling texture.");
    }
    currentTexture = texture;
  }
}


void Renderer::renderFacesForMaterial(Model* model, unsigned int frameNum,
    Material* material, RenderStyle style)
{
  Frame& frame = model->frames[frameNum];
  for (unsigned int f = 0; f < frame.faces.size(); ++f) {
    Face& face = *frame.faces[f];
    if (face.material != material)
      continue;

    switch (style) {
    case kLines:
      glBegin(GL_LINE_LOOP);
      break;
    case kPolygons:
    default:
      glBegin(GL_POLYGON);
      break;
    }

    for (unsigned int i = 0; i < face.size(); ++i) {
      if (face.material != NULL) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, face.material->Ka.data);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face.material->Kd.data);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, face.material->Ks.data);

        if (face[i].vt >= 0 && (unsigned int)face[i].vt < frame.vt.size()) {
          Float4& vt = frame.vt[face[i].vt];
          if (face.material->mapKa != NULL)
            glMultiTexCoord3f(GL_TEXTURE0, vt.x, vt.y, vt.z);
          if (face.material->mapKd != NULL)
            glMultiTexCoord3f(GL_TEXTURE1, vt.x, vt.y, vt.z);
          if (face.material->mapKs != NULL)
            glMultiTexCoord3f(GL_TEXTURE2, vt.x, vt.y, vt.z);
        }
      } else {
        float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
      }
      
      if (face[i].vn >= 0 && (unsigned int)face[i].vn < frame.vn.size()) {
        Float4& vn = frame.vn[face[i].vn];
        glNormal3f(vn.x, vn.y, vn.z);
      }

      Float4& v = frame.v[face[i].v];
      glVertex4f(v.x, v.y, v.z, v.w);
    }

    glEnd();
    checkGLError("Error while drawing face.");
  }
}


void Renderer::loadTexturesForModel(Model* model)
{
  if (model == NULL)
    return;

  std::map<std::string, Material>::iterator iter;
  for (iter = model->materials.begin(); iter != model->materials.end(); ++iter) {
    loadTexture(iter->second.mapKa);
    loadTexture(iter->second.mapKd);
    loadTexture(iter->second.mapKs);
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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


void Renderer::drawLight(const Float4& pos)
{
  glBegin(GL_LINES);
  glVertex3f(pos.x - 0.5, pos.y, pos.z);
  glVertex3f(pos.x + 0.5, pos.y, pos.z);

  glVertex3f(pos.x, pos.y - 0.5, pos.z);
  glVertex3f(pos.x, pos.y + 0.5, pos.z);
  
  glVertex3f(pos.x, pos.y, pos.z - 0.5);
  glVertex3f(pos.x, pos.y, pos.z + 0.5);
  glEnd();
}


void Renderer::drawFPSCounter(int width, int height, float fps)
{
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
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

