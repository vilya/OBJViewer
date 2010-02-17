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
size_t systemTimeInMilliseconds();


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

RenderGroup::RenderGroup(Material* iMaterial, RenderGroupType iType, GLuint iShaderProgramID) :
  _material(iMaterial),
  _type(iType),
  _size(0),
  _hasColors(false),
  _currentTime(-1e20),
  _coords(),
  _bufferID(0),
  _indexesID(0),
  _shaderProgramID(iShaderProgramID)
{
}


Material* RenderGroup::getMaterial() const
{
  return _material;
}


void RenderGroup::add(Model* model, Face* face)
{
  if (_size == 0) {
    _hasColors = (*face)[0].c >= 0;
  }

  for (size_t i = 0; i < face->size(); ++i) {
    int vi = (*face)[i].v;
    _coords.push_back(&model->v[vi]);

    int vti = (*face)[i].vt;
    _texCoords.push_back(&model->vt[vti]);

    int vni = (*face)[i].vn;
    _normals.push_back(&model->vn[vni]);

    if (_hasColors) {
      int ci = (*face)[i].c;
      _colors.push_back(&model->colors[ci]);
    }

    ++_size;
  }
}


size_t RenderGroup::size() const
{
  return _size;
}


size_t RenderGroup::floatsPerVertex() const
{
  // First 9 floats are: x, y, z, u, v, nx, ny, nz, nw.
  return 9 + (_hasColors ? 3 : 0);
}


void RenderGroup::flipNormals()
{
  _flipNormals = !_flipNormals;
  _currentTime = -1.0; // force the next setTime call to recalculate.
}


void RenderGroup::prepare()
{
  size_t bufferSize = _coords.size() * sizeof(float) * floatsPerVertex();

  // Get a buffer ID for the coords & allocate space for them. 
  glGenBuffers(1, &_bufferID);
  glBindBuffer(GL_ARRAY_BUFFER, _bufferID);
  glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STREAM_DRAW);
  checkGLError("Error setting up vertex buffer");

  // Get a buffer ID for the indexes, upload them and clear out the local copy.
  glGenBuffers(1, &_indexesID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      sizeof(GLuint) * _size, NULL, GL_STATIC_DRAW);
  checkGLError("Error setting up index buffer");

  GLuint* indexBuffer = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  for (GLuint i = 0; i < _size; ++i)
    indexBuffer[i] = i;
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  checkGLError("Error filling index buffer");
}


void RenderGroup::render(float time)
{
  checkGLError("Error before RenderGroup::render");
  glBindBuffer(GL_ARRAY_BUFFER, _bufferID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);

  glEnableClientState(GL_VERTEX_ARRAY);
  checkGLError("Error before setTime");
  setTime(time);
  checkGLError("Error in setTime.");

  // Set up the shaders for this render group.
  setupShaders();

  // Now start the rendering.
  GLuint stride = sizeof(float) * floatsPerVertex();
  GLuint offset = 0;

  glVertexPointer(3, GL_FLOAT, stride, (const GLvoid*)offset);
  offset += 3 * sizeof(float);
  checkGLError("Error setting up vertex pointer");

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
    if (textures[i] != NULL) {
      glEnable(GL_TEXTURE_2D);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindTexture(GL_TEXTURE_2D, textures[i]->getTexID());
      glTexCoordPointer(2, GL_FLOAT, stride, (const GLvoid*)offset);
    } else {
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
  }
  offset += 2 * sizeof(float);
  checkGLError("Error setting up textures.");

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, stride, (const GLvoid*)offset);
  offset += 4 * sizeof(float);
  checkGLError("Error setting up normals.");

  if (_hasColors) {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, stride, (const GLvoid*)offset);
    offset += 3 * sizeof(float);
  } else if (_material != NULL) {
    glDisableClientState(GL_COLOR_ARRAY);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _material->Ka.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _material->Kd.data);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _material->Ks.data);
  } else {
    glDisableClientState(GL_COLOR_ARRAY);
    float defaultColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultColor);
  }
  checkGLError("Error setting up colors.");

  if (_material != NULL) {
    float shininess = std::min(_material->Ns * 128.0, 128.0);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  } else {
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);
  }
  checkGLError("Error setting up shininess.");
 
  switch (_type) {
    case kTriangleGroup:
      glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);
      break;
    case kPolygonGroup:
      glDrawElements(GL_POLYGON, _size, GL_UNSIGNED_INT, 0);
      break;
  }
  checkGLError("Error drawing elements.");

  glDisableClientState(GL_VERTEX_ARRAY);
  for (int i = 0; i < 4; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glClientActiveTexture(GL_TEXTURE0 + i);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  if (_hasColors)
    glDisableClientState(GL_COLOR_ARRAY);

  // Release the VBOs.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  checkGLError("Error clearing up.");
}


void RenderGroup::renderPoints(float time)
{
  glBindBuffer(GL_ARRAY_BUFFER, _bufferID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glDisable(GL_LIGHTING);
  glEnableClientState(GL_VERTEX_ARRAY);

  setTime(time);

  GLuint stride = sizeof(float) * floatsPerVertex();
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


void RenderGroup::renderLines(float time)
{
  glBindBuffer(GL_ARRAY_BUFFER, _bufferID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexesID);
  glDisable(GL_LIGHTING);
  glEnableClientState(GL_VERTEX_ARRAY);

  setTime(time);

  GLuint stride = sizeof(float) * floatsPerVertex();
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


void RenderGroup::setTime(float time)
{
  // Note: This function assumes that the correct vertex buffer has already been bound.

  if (time == _currentTime)
    return;

  _currentTime = time;

  // Calculate the current animation frame.
  size_t vertexSize = floatsPerVertex();
  float* vertexBuffer = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  float* vertexBufferPos = vertexBuffer;
  for (size_t i = 0; i < _coords.size(); ++i) {
    Float4 coord = _coords[i]->valueAt(time);
    vertexBufferPos[0] = coord.x;
    vertexBufferPos[1] = coord.y;
    vertexBufferPos[2] = coord.z;
    vertexBufferPos += vertexSize;
  }
  vertexBuffer += 3;
  vertexBufferPos = vertexBuffer;
  for (size_t i = 0; i < _texCoords.size(); ++i) {
    Float4 texCoord = _texCoords[i]->valueAt(time);
    vertexBufferPos[0] = texCoord.x;
    vertexBufferPos[1] = texCoord.y;
    vertexBufferPos += vertexSize;
  }
  vertexBuffer += 2;
  vertexBufferPos = vertexBuffer;
  if (!_flipNormals) {
    for (size_t i = 0; i < _normals.size(); ++i) {
      Float4 normal = _normals[i]->valueAt(time);
      vertexBufferPos[0] = normal.x;
      vertexBufferPos[1] = normal.y;
      vertexBufferPos[2] = normal.z;
      vertexBufferPos[3] = normal.w;
      vertexBufferPos += vertexSize;
    }
  } else {
    for (size_t i = 0; i < _normals.size(); ++i) {
      Float4 normal = _normals[i]->valueAt(time);
      vertexBufferPos[0] = -normal.x;
      vertexBufferPos[1] = -normal.y;
      vertexBufferPos[2] = -normal.z;
      vertexBufferPos[3] = normal.w;
      vertexBufferPos += vertexSize;
    }
  }
  vertexBuffer += 4;
  if (_hasColors) {
    vertexBufferPos = vertexBuffer;
    for (size_t i = 0; i < _colors.size(); ++i) {
      Float4 color = _colors[i]->valueAt(time);
      vertexBufferPos[0] = color.r;
      vertexBufferPos[1] = color.g;
      vertexBufferPos[2] = color.b;
      vertexBufferPos += vertexSize;
    }
    vertexBuffer += 3;
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
}


void RenderGroup::setupShaders()
{
  glUseProgram(_shaderProgramID);
  checkGLError("Error setting up shaders (bad program ID?)");

  if (_material != NULL) {
    RawImage* textures[] = { _material->mapKa, _material->mapKd, _material->mapKs, _material->mapD };
    const char* names[] = { "mapKa", "mapKd", "mapKs", "mapD" };
    const char* flagNames[] = { "hasMapKa", "hasMapKd", "hasMapKs", "hasMapD" };

    for (size_t i = 0; i < 4; ++i) {
      if (textures[i] != NULL) {
        glUniform1i(glGetUniformLocation(_shaderProgramID, names[i]), i);
        checkGLError("Error setting up texture for shader");

        glUniform1i(glGetUniformLocation(_shaderProgramID, flagNames[i]), 1);
        checkGLError("Error setting up texture flag for shader");
      }
    }
  }
}


//
// Renderer METHODS
//

Renderer::Renderer(Camera* camera, Model* model, size_t maxTextureWidth, size_t maxTextureHeight) :
  _headlightType(kDirectional),
  _drawPolys(true),
  _drawPoints(false),
  _drawLines(false),
  _model(model),
  _camera(camera),
  _maxTextureWidth(maxTextureWidth),
  _maxTextureHeight(maxTextureHeight),
  _renderGroups(),
  _currentMapKa(NULL),
  _currentMapKd(NULL),
  _currentMapKs(NULL),
  _currentMapD(NULL),
  _fps(),
  _shaderWithMaterial(0),
  _shaderNoMaterial(0),
  _currentTime(0),
  _playing(false),
  _since(0)
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


void Renderer::prepare()
{
  prepareModel();
  prepareShaders();
  prepareRenderGroups();
  prepareMaterials();

  loadTextures(_renderGroups);
  _camera->frontView(_model->low, _model->high);
}


void Renderer::render(int width, int height)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // If we're playing the animation, calculate the new frame time.
  if (_playing)
    setTime(calculatePlaybackTime());

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
        group->render(_currentTime);
      }
    }

    if (_drawPoints) {
      glDisable(GL_LIGHTING);
      for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
        RenderGroup* group = *iter;
        group->renderPoints(_currentTime);
      }
      glEnable(GL_LIGHTING);
    }

    if (_drawLines) {
      glDisable(GL_LIGHTING);
      for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter) {
        RenderGroup* group = *iter;
        group->renderLines(_currentTime);
      }
      glEnable(GL_LIGHTING);
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


void Renderer::setTime(float time)
{
  time = fmodf(time, _model->numKeyframes());
  if (time < 0)
    time += _model->numKeyframes();
  _currentTime = time;
}


void Renderer::nextFrame()
{
  setTime(fmodf(_currentTime + 1.0, _model->numKeyframes()));
}


void Renderer::previousFrame()
{
  if (_currentTime < 1.0)
    setTime(_currentTime + _model->numKeyframes() - 1.0);
  else
    setTime(_currentTime - 1.0);
}


void Renderer::firstFrame()
{
  _currentTime = 0.0;
}


void Renderer::lastFrame()
{
  _currentTime = _model->numKeyframes() - 1;
}


void Renderer::togglePlaying()
{
  _playing = !_playing;
  if (_playing)
    _since = glutGet(GLUT_ELAPSED_TIME);
}


void Renderer::flipNormals()
{
  std::list<RenderGroup*>::iterator iter;
  for (iter = _renderGroups.begin(); iter != _renderGroups.end(); ++iter)
    (*iter)->flipNormals();
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


void Renderer::prepareModel()
{
  // Fill in default texture coordinates where necessary.
  size_t defaultTexCoordIndex = _model->vt.size();
  _model->addVt(Float4(0.5, 0.5, 0.0, 1.0));
  for (size_t i = 0; i < _model->faces.size(); ++i) {
    Face& face = *_model->faces[i];
    for (size_t vertexNum = 0; vertexNum < face.size(); ++vertexNum) {
      if (face[vertexNum].vt < 0)
        face[vertexNum].vt = defaultTexCoordIndex;
    }
  }

  // Calculate the normals if they're not present.
  if (_model->vn.size() == 0) {
    fprintf(stderr, "Calculating normals...\n");

    // Setup a normal of 0,0,0,0 for all keyframes.
    while (_model->vn.size() < _model->v.size()) {
      Curve curve;
      while (curve.numKeyframes() < _model->numKeyframes())
        curve.addKeyframe(Float4(0, 0, 0, 0));
      _model->vn.push_back(curve);
    }

    for (size_t i = 0; i < _model->faces.size(); ++i) {
      Face& face = *_model->faces[i];
      for (size_t frame = 0; frame < _model->numKeyframes(); ++frame) {
        const Float4& a = _model->v[face[0].v][frame];
        const Float4& b = _model->v[face[1].v][frame];
        const Float4& c = _model->v[face[2].v][frame];
        Float4 faceNormal = normalize(cross(b - a, c - a));

        for (size_t j = 0; j < face.size(); ++j) {
          Curve& curve = _model->vn[face[j].v];
          curve[frame] = curve[frame] + faceNormal;
        }
      }
      for (size_t j = 0; j < face.size(); ++j)
        face[j].vn = face[j].v;
    }

    for (size_t i = 0; i < _model->vn.size(); ++i) {
      for (size_t frame = 0; frame < _model->numKeyframes(); ++frame)
        _model->vn[i][frame] = normalize(_model->vn[i][frame]);
    }
  }

  // Count the animated points.
  if (_model->numKeyframes() > 1) {
    size_t totalPoints = 0;
    size_t animatedPoints = 0;

    Float4 size = _model->high - _model->low;
    for (size_t i = 0; i < _model->v.size(); ++i) {
      Curve& curve = _model->v[i];
      const Float4 initialPos = curve[0];
      for (size_t keyFrame = 1; keyFrame < curve.numKeyframes(); ++keyFrame) {
        const Float4 keyPos = curve[keyFrame];
        // If a vertex has moved by more than 1% of the total object size...
        if (lengthSqr( (keyPos - initialPos) / size ) > 1e-4)
          ++animatedPoints; 
        ++totalPoints;
      }
    }

    fprintf(stderr, "%ld of %ld points (%1.2f%%) are animated.\n",
        animatedPoints, totalPoints,
        100.0 * float(animatedPoints) / float(totalPoints));
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
      (*groupMap)[material].push_back(new RenderGroup(material, type,
          material ? _shaderWithMaterial : _shaderNoMaterial));
    }

    std::list<RenderGroup*>& groups = (*groupMap)[material];
    if ((!isTriangle && groups.front()->size() > 0) || groups.front()->size() >= MAX_FACES_PER_VBO)
      groups.push_front(new RenderGroup(material, type,
          material ? _shaderWithMaterial : _shaderNoMaterial));
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
    if (material->d != 1) {
      material->Ka.a = material->d;
      material->Kd.a = material->d;
      material->Ks.a = material->d;
    }
  }
}


void Renderer::prepareShaders()
{
  GLuint vertexShader, fragmentShader;

  // Set up the shader program for objects with a material.
  vertexShader = loadShader(GL_VERTEX_SHADER, "vertex.vert");
  fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment.frag");
  if (vertexShader != 0 && fragmentShader != 0)
    _shaderWithMaterial = linkProgram(vertexShader, fragmentShader);

  // Set up the shader program for objects without a material.
  vertexShader = loadShader(GL_VERTEX_SHADER, "vertex-notexture.vert");
  fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment-notexture.frag");
  if (vertexShader != 0 && fragmentShader != 0)
    _shaderNoMaterial = linkProgram(vertexShader, fragmentShader);
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
  fprintf(stderr, "Loading %dx%d %d bpp texture onto the GPU.\n",
      tex->getWidth(), tex->getHeight(), tex->getBytesPerPixel() * 8);

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

    sprintf(buf, "Frame %0.1f of %ld", _currentTime, _model->numKeyframes());
    drawRightAlignedBitmapString(width - 10, 10, GLUT_BITMAP_8_BY_13, buf);
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


void Renderer::drawRightAlignedBitmapString(float x, float y, void* font, char* str)
{
  float xPos = x;
  while (*str != '\0') {
    char* ch = str;
    for (ch = str; *ch != '\0' && *ch != '\n'; ++ch)
      xPos -= glutBitmapWidth(font, *ch);
    for (; str < ch; ++str) {
      glRasterPos2f(xPos, y);
      glutBitmapCharacter(font, *str);
      xPos += glutBitmapWidth(font, *str);
    }
    if (*str == '\n') {
      ++str;
      xPos = x;
      y -= 15;
    }
  }
}


GLuint Renderer::loadShader(GLenum shaderType, const char* path)
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

  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID, 1, (const GLchar**)&text, NULL);
  glCompileShader(shaderID);

  GLint status;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    fprintf(stderr, "Shader %s failed to compile:\n", path);
    printShaderInfoLog(shaderID);
    shaderID = 0;
  }

  delete[] text;
  return shaderID;
}


GLuint Renderer::linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID)
{
  GLuint programID = glCreateProgram();
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);
  glLinkProgram(programID);

  GLint status;
  glGetProgramiv(programID, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    fprintf(stderr, "Shader program failed to link.\n");
    printProgramInfoLog(programID);
    programID = 0;
  }

  return programID;
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


float Renderer::calculatePlaybackTime()
{
  int now = glutGet(GLUT_ELAPSED_TIME);
  float incr = 24.0 * float(now - _since) / 1000.0;
  // Clamp incr to 1.0 so that we always see every frame, even when our frame
  // rate for display is less than 24 fps.
  if (incr > 1.0)
    incr = 1.0;
  _since = now;
  return _currentTime + incr;
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

