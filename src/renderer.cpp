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
// Renderer METHODS
//

Renderer::Renderer(Model* model) :
  _style(kPolygons),
  _model(model),
  _camU(0),
  _camV(0),
  _camDist(10),
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

  loadTexturesForModel(_model);
}


Renderer::~Renderer()
{
}


void Renderer::moveCameraBy(float u, float v, float distance)
{
  _camU = fmodf(_camU + u, 360.0f);
  _camV = fmodf(_camV + v, 360.0f);
  _camDist += distance;
}


void Renderer::moveCameraTo(float u, float v, float distance)
{
  _camU = fmodf(u, 360.0f);
  _camV = fmodf(v, 360.0f);;
  _camDist = distance;
}


void Renderer::setStyle(RenderStyle style)
{
  _style = style;
}


void Renderer::render(int width, int height)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Position the camera
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glViewport(0, 0, width, height); // Set the viewport to be the entire window

  // Set up the camera position
  gluPerspective(30, double(width) / double(height), 0.1, 100.0);
  gluLookAt(
      0, 0, _camDist, // Camera position
      0, 0, 0,        // target
      0, 1, 0         // up
  );
  glRotatef(_camU, 0, 1, 0);
  glRotatef(_camV, 1, 0, 0);

  // Draw the scene.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Put a light at the same position as the camera.
  float light[4] = { 0, 0, _camDist, 1 };
  glPushMatrix();
  glRotatef(-_camV, 1, 0, 0);
  glRotatef(-_camU, 0, 1, 0);
  glLightfv(GL_LIGHT0, GL_POSITION, light);
  glPopMatrix();

  drawModel(_model, _style == kPolygons);
  drawFPSCounter(width, height, _fps.fps());

  glutSwapBuffers();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  _fps.increment();
}


void Renderer::loadTexturesForModel(Model* model)
{
  if (model == NULL)
    return;

  unsigned int texId = 10;
  std::map<std::string, Material>::iterator iter;
  for (iter = model->materials.begin(); iter != model->materials.end(); ++iter) {
    Image* tex = iter->second.mapKd;
    if (tex != NULL) {
      loadTexture(GL_TEXTURE0, texId, *tex);
      ++texId;
    }
  }
}


void Renderer::drawModel(Model* theModel, bool filledPolygons)
{
  if (theModel == NULL) {
    if (filledPolygons)
      glutSolidTeapot(1.0f);
    else
      glutWireTeapot(1.0f);
  } else {
    Image* currentKd = NULL;
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);

    std::map<std::string, Material>::iterator m;
    for (m = theModel->materials.begin(); m != theModel->materials.end(); ++m) {
      Material *currentMat = &m->second;

      if (currentMat->mapKd != currentKd) {
        glActiveTexture(GL_TEXTURE0);
        checkGLError("No luck activating texture 0");
        if (currentMat->mapKd != NULL) {
          loadTexture(GL_TEXTURE0, currentMat->mapKd->getTexID(), *currentMat->mapKd);
          checkGLError("Error loading texture 0");
        } else {
          glDisable(GL_TEXTURE_2D);
          checkGLError("Error disabling texture 0");
        }
        currentKd = currentMat->mapKd;
      }

      for (unsigned int f = 0; f < theModel->faces.size(); ++f) {
        Face& face = *theModel->faces[f];
        if (face.material != currentMat)
          continue;

        if (filledPolygons)
          glBegin(GL_POLYGON);
        else
          glBegin(GL_LINE_LOOP);
 
        for (unsigned int i = 0; i < face.size(); ++i) {
          if (face.material != NULL) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, face.material->Ka.data);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face.material->Kd.data);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, face.material->Ks.data);

            if (face.material->mapKd != NULL && face[i].vt >= 0) {
              Float4& vt = theModel->vt[face[i].vt];
              glMultiTexCoord3f(GL_TEXTURE0, vt.x, vt.y, vt.z);
            }
          } else {
            float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
          }
          
          if (face[i].vn >= 0) {
            Float4& vn = theModel->vn[face[i].vn];
            glNormal3f(vn.x, vn.y, vn.z);
          }

          Float4& v = theModel->v[face[i].v];
          glVertex4f(v.x, v.y, v.z, v.w);
        }

        glEnd();
        checkGLError("Error while drawing face.");
      }
    }
  }
}


void Renderer::loadTexture(GLenum texUnit, int texID, Image& tex)
{
  GLenum targetType;
  switch (tex.getBytesPerPixel()) {
  case 1:
    targetType = GL_ALPHA;
    break;
  case 3:
    targetType = GL_RGB;
    break;
  case 4:
    targetType = GL_RGBA;
    break;
  default:
    targetType = GL_RGB;
    break;
  }
  checkGLError("Some GL error before loading texture.");

  glActiveTexture(texUnit);
  checkGLError("Texture unit not active.");
  glEnable(GL_TEXTURE_2D);
  checkGLError("Textures not enabled.");
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
  glTexImage2D(GL_TEXTURE_2D, 0, targetType, tex.getWidth(), tex.getHeight(), 0,
               tex.getType(), GL_UNSIGNED_BYTE, tex.getPixels());
  checkGLError("Texture failed to load.");

  tex.setTexID(texID);
}


void Renderer::drawFPSCounter(int width, int height, float fps)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width, 0, height, -1, 1);
  //glScalef(1, -1, 1);
  //glTranslatef(0, -height, 0);

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

