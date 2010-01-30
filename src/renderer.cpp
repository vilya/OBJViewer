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


void Camera::centerView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.z - frame.low.z) / 10.0;
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::frontView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.z - frame.low.z) / 2 +
      distanceFrom(frame.high.x, frame.low.x, frame.high.y, frame.low.y);
  _rotation = Float4(0, 0, 0, distance);
}


void Camera::backView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.z - frame.low.z) / 2 +
      distanceFrom(frame.high.x, frame.low.x, frame.high.y, frame.low.y);
  _rotation = Float4(0, 180, 0, distance);
}


void Camera::leftView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.x - frame.low.x) / 2 +
      distanceFrom(frame.high.z, frame.low.z, frame.high.y, frame.low.y);
  _rotation = Float4(0, 270, 0, distance);
}


void Camera::rightView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.x - frame.low.x) / 2 +
      distanceFrom(frame.high.z, frame.low.z, frame.high.y, frame.low.y);
  _rotation = Float4(0, 90, 0, distance);
}


void Camera::topView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.y - frame.low.y) +
      distanceFrom(frame.high.x, frame.low.x, frame.high.z, frame.low.z);
  _rotation = Float4(90, 0, 0, distance);
}


void Camera::bottomView(Model* model, unsigned int frameNum)
{
  Frame& frame = model->frames[frameNum];

  _target = (frame.high + frame.low) / 2;
  float distance = (frame.high.y - frame.low.y) +
      distanceFrom(frame.high.x, frame.low.x, frame.high.z, frame.low.z);
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

RenderGroup::RenderGroup(Material* iMaterial, int iFirstID, int iLastID) :
  mat(iMaterial), firstID(iFirstID), lastID(iLastID)
{
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
    prepare();

    _camera->frontView(_model);
    loadTexturesForModel(_model);
    drawModel(_model, 0, kLines);
    drawModel(_model, 0, kPolygons);
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
  Frame& frame = _model->frames[0];
  _camera->setup(width, height, frame.low, frame.high);

  // Draw the scene.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Put a light at the same position as the camera.
  headlight(GL_LIGHT0, Float4(1, 1, 1, 1));

  _camera->transformTo();
  if (_model != NULL) {
    std::list<RenderGroup>::iterator iter;
    for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
      RenderGroup& group = *iter;
      setupMaterial(group.mat);
      for (int displayListID = group.firstID; displayListID < group.lastID; ++displayListID)
        glCallList(displayListID);
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
  Frame& frame = _model->frames[0];

  std::map<std::string, Material>::iterator m;
  for (m = _model->materials.begin(); m != _model->materials.end(); ++m) {
    Material* material = &m->second;
    std::vector<Face*> faces;
    for (size_t i = 0; i < frame.faces.size(); ++i) {
      Face* face = frame.faces[i];
      if (face->material == material)
        faces.push_back(face);
    }

    if (faces.size() == 0)
      continue;

    size_t numRenderGroups = faces.size() / MAX_FACES_PER_DISPLAYLIST;
    if (faces.size() % MAX_FACES_PER_DISPLAYLIST != 0)
        ++numRenderGroups;

    GLuint firstID = glGenLists(numRenderGroups);
    _renderGroups.push_back(RenderGroup(material, firstID, firstID + numRenderGroups));
  }
}


void Renderer::drawModel(Model* model, unsigned int frameNum, RenderStyle style)
{
  glDisable(GL_TEXTURE_2D);

  _currentMapKa = NULL;
  _currentMapKd = NULL;
  _currentMapKs = NULL;

  std::list<RenderGroup>::iterator iter;
  for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
    RenderGroup& rg = *iter;
    setupMaterial(rg.mat);
    renderFacesForMaterial(model, frameNum, style, rg);
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


void Renderer::renderFacesForMaterial(Model* model, unsigned int frameNum,
    RenderStyle style, const RenderGroup& group)
{
  Frame& frame = model->frames[frameNum];
  Material* material = group.mat;

  if (material != NULL) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->Ka.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->Kd.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->Ks.data);
  } else {
    float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
  }

  int displayListID = group.firstID;
  size_t facesRendered = 0;
  for (unsigned int f = 0; f < frame.faces.size(); ++f) {
    if (facesRendered % MAX_FACES_PER_DISPLAYLIST == 0) {
      if (facesRendered > 0)
        glEndList();
      glNewList(displayListID, GL_COMPILE);
      ++displayListID;
    }

    Face& face = *frame.faces[f];
    if (face.material != material)
      continue;

    glBegin( (style == kLines) ? GL_LINE_LOOP : GL_POLYGON );

    for (unsigned int i = 0; i < face.size(); ++i) {
      if (material != NULL && face[i].vt >= 0 && (unsigned int)face[i].vt < frame.vt.size()) {
        Float4& vt = frame.vt[face[i].vt];
        if (material->mapKa != NULL)
          glMultiTexCoord3f(GL_TEXTURE0, vt.x, vt.y, vt.z);
        if (material->mapKd != NULL)
          glMultiTexCoord3f(GL_TEXTURE1, vt.x, vt.y, vt.z);
        if (material->mapKs != NULL)
          glMultiTexCoord3f(GL_TEXTURE2, vt.x, vt.y, vt.z);
      }
      
      if (face[i].vn >= 0 && (unsigned int)face[i].vn < frame.vn.size()) {
        Float4& vn = frame.vn[face[i].vn];
        glNormal3f(vn.x, vn.y, vn.z);
      }

      Float4& v = frame.v[face[i].v];
      glVertex4f(v.x, v.y, v.z, v.w);
    }

    glEnd();
  }
  glEndList();
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

