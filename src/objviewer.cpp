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

void checkGLError(const char *errMsg, const char *okMsg = NULL);

    
//
// OBJViewerApp METHODS
//

OBJViewerApp::OBJViewerApp(int argc, char **argv) :
    winX(100), winY(100), winWidth(800), winHeight(600),
    fullscreen(false),
    mouseX(0), mouseY(0),
    xRot(0.0f), yRot(0.0f),
    polygons(true),
    model(NULL),
    modelDisplayList(0xFFFFFFFF),
    linesDisplayList(0xFFFFFFFF)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(winX, winY);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Vil's OBJ Viewer");

    processArgs(argc, argv);
    init();

    glutDisplayFunc(doRender);
    glutReshapeFunc(doResize);
    glutKeyboardFunc(doKeyPressed);
    glutMouseFunc(doMousePressed);
    glutMotionFunc(doMouseDragged);
}


OBJViewerApp::~OBJViewerApp()
{
    cleanUp();
}


void OBJViewerApp::renderScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();

  drawModel(model, polygons);
  /*
  if (polygons)
    glCallList(modelDisplayList);
  else
    glCallList(linesDisplayList);
  */

  glPopMatrix();
  glutSwapBuffers();
}


void OBJViewerApp::changeSize(int width, int height)
{
    if (!fullscreen) {
        winWidth = width;
        winHeight = height;
    }

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set the viewport to be the entire window
    glViewport(0, 0, width, height);

    // Set up the camera position
    gluPerspective(30, double(winWidth) / double(winHeight), 0.1, 100.0);
    gluLookAt(0, 0, -10, 0, 0, 0, 0, 1, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void OBJViewerApp::keyPressed(unsigned char key, int x, int y)
{
    switch (key) {
        case 27: // 27 is the ESC key.
            cleanUp();
            exit(0);
            break;
        case 'r':
            cleanUp();
            init();
            renderScene();
            break;
        case 'f':
            fullscreen = !fullscreen;
            if (fullscreen)
                glutFullScreen();
            else
                glutReshapeWindow(winWidth, winHeight);
            break;
        case 'p':
            polygons = !polygons;
            glutPostRedisplay();
        default:
            break;
    }
}


void OBJViewerApp::mousePressed(int button, int state, int x, int y) {
    //fprintf(stderr, "MOUSE_PRESSED(button=%d, state=%d, x=%d, y=%d)\n", button, state, x, y);
    mouseX = x;
    mouseY = y;
}


void OBJViewerApp::mouseDragged(int x, int y) {
    //fprintf(stderr, "MOUSE_DRAGGED(x=%d, y=%d)\n", x, y);
    int dx = x - mouseX;
    int dy = y - mouseY;
    glRotatef(dx, 0, 1, 0);
    glRotatef(dy, 1, 0, 0);

    mouseY = y;
    mouseX = x;
    glutPostRedisplay();
}


void OBJViewerApp::run()
{
    glutMainLoop();
}


void OBJViewerApp::init()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_TEXTURE_2D);

  float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glShadeModel(GL_SMOOTH);

  float position[4] = { 0, 0, -10, 0 };
  glLightfv(GL_LIGHT0, GL_POSITION, position);

  loadTexturesForModel(model);

  //modelDisplayList = glGenLists(2);
  //linesDisplayList = modelDisplayList + 1;

  /*
  glNewList(modelDisplayList, GL_COMPILE);
  drawModel(model, true);
  glEndList();

  glNewList(linesDisplayList, GL_COMPILE);
  drawModel(model, false);
  glEndList();
  */
}


void OBJViewerApp::loadTexturesForModel(Model* theModel)
{
  if (theModel == NULL)
    return;

  unsigned int texId = 10;
  std::map<std::string, Material>::iterator iter;
  for (iter = theModel->materials.begin(); iter != theModel->materials.end(); ++iter) {
    Image* tex = iter->second.mapKa;
    if (tex != NULL) {
      loadTexture(*tex, texId);
      ++texId;
    }

    tex = iter->second.mapKd;
    if (tex != NULL) {
      loadTexture(*tex, texId);
      ++texId;
    }

    tex = iter->second.mapKd;
    if (tex != NULL) {
      loadTexture(*tex, texId);
      ++texId;
    }
  }
}


void OBJViewerApp::drawModel(Model* theModel, bool filledPolygons)
{
  float left = -0.5f;
  float right = 0.5f;
  float bottom = -0.5f;
  float top = 0.5f;

  float width = right - left;
  float height = top - bottom;

  if (theModel == NULL) {
    if (filledPolygons)
      glutSolidTeapot(fminf(width, height));
    else
      glutWireTeapot(fminf(width, height));
  } else {
    Image* currentKa = NULL;
    Image* currentKd = NULL;
    Image* currentKs = NULL;

    std::map<std::string, Material>::iterator m;
    for (m = theModel->materials.begin(); m != theModel->materials.end(); ++m) {
      Material *currentMat = &m->second;

      if (currentMat->mapKa != NULL && currentMat->mapKa != currentKa) {
        glActiveTexture(GL_TEXTURE0);
        checkGLError("Couldn't activate ambient texture");
        glBindTexture(GL_TEXTURE_2D, currentMat->mapKa->getTexID());
        checkGLError("Couldn't bind Ka");
        currentKa = currentMat->mapKa;
      }
      if (currentMat->mapKd != NULL && currentMat->mapKd != currentKd) {
        glActiveTexture(GL_TEXTURE0);
        checkGLError("Couldn't activate diffuse texture");
        glBindTexture(GL_TEXTURE_2D, currentMat->mapKd->getTexID());
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
          fprintf(stderr, "Unable to bind texture %d\n", currentMat->mapKd->getTexID());
        else
          fprintf(stderr, "Bound texture %d\n", currentMat->mapKd->getTexID());
        //checkGLError("Couldn't bind Kd");
        currentKd = currentMat->mapKd;
      }
      if (currentMat->mapKs != NULL && currentMat->mapKs != currentKs) {
        glActiveTexture(GL_TEXTURE2);
        checkGLError("Couldn't activate specular texture");
        glBindTexture(GL_TEXTURE_2D, currentMat->mapKs->getTexID());
        checkGLError("Couldn't bind Ks");
        currentKs = currentMat->mapKs;
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
            //glColor3fv(face.material->Kd.data);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, face.material->Ka.data);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face.material->Kd.data);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, face.material->Ks.data);
            //glMaterialf(GL_FRONT, GL_SHININESS, face.material->Ns);
          } else {
            float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
          }
          checkGLError("material properties are buggered.");

          if (face[i].vt >= 0) {
            Float4& vt = theModel->vt[face[i].vt];
            if (face.material->mapKa != NULL) {
              glMultiTexCoord3f(GL_TEXTURE0, vt.x, vt.y, vt.z);
              checkGLError("mapKa");
            }
            if (face.material->mapKd != NULL) {
              glTexCoord3f(vt.x, vt.y, vt.z);
              checkGLError("mapKd");
            }
            if (face.material->mapKs != NULL) {
              glMultiTexCoord3f(GL_TEXTURE2, vt.x, vt.y, vt.z);
              checkGLError("mapKs");
            }
          }
          
          if (face[i].vn >= 0) {
            Float4& vn = theModel->vn[face[i].vn];
            glNormal3f(vn.x, vn.y, vn.z);
            checkGLError("normals");
          }

          Float4& v = theModel->v[face[i].v];
          glVertex4f(v.x, v.y, v.z, v.w);
          checkGLError("vertexes");
        }

        glEnd();
      }
    }
  }
}


void OBJViewerApp::loadTexture(Image& tex, int texID)
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

  fprintf(stderr, "Loading texture %d\n", texID);
  glEnable(GL_TEXTURE_2D);
  checkGLError("Textures not enabled.", "Textures enabled");
  glBindTexture(GL_TEXTURE_2D, texID);
  checkGLError("Texture not bound.", "Texture bound.");
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  checkGLError("Pixel storage format not set.", "Pixel storage format set.");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  checkGLError("Texture paramaters not set.", "Texture parameters set.");
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, targetType, tex.getWidth(), tex.getHeight(), 0,
               tex.getType(), GL_UNSIGNED_BYTE, tex.getPixels());
  checkGLError("Texture failed to load.", "Texture loaded successfully.");

  tex.setTexID(texID);
}


void OBJViewerApp::usage(char *progname)
{
    fprintf(stderr,
"Usage: %s [options] <objfile>\n"
"\n"
"Where [options] can be any combination of:\n"
"  -v,--verbose                 Print out some extra information.\n"
"  -h,--help                    Print this message and exit.\n"
            , basename(progname));
}


void OBJViewerApp::processArgs(int argc, char **argv)
{
    const char *short_opts = "h";
    struct option long_opts[] = {
        { "help",               no_argument,        NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };
    int ch;
    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (ch) {
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
    if (argc > 0) {
        try {
            model = loadModel(argv[0]);
        } catch (ParseException& e) {
            fprintf(stderr, "Unable to load model. Continuing with default model.\n");
        }
    }
}


void OBJViewerApp::cleanUp()
{
}


void doRender()
{
    app->renderScene();
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


void checkGLError(const char *errMsg, const char *okMsg)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "%s: %s (%d)\n", errMsg, gluErrorString(err), err);
  } else if (okMsg != NULL) {
    fprintf(stderr, "%s\n", okMsg);
  }
}


int main(int argc, char **argv)
{
    app = new OBJViewerApp(argc, argv);
    app->run();
    delete app;
    return 0;
}

