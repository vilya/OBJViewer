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

    
//
// OBJViewerApp METHODS
//

OBJViewerApp::OBJViewerApp(int argc, char **argv) :
    winX(100), winY(100), winWidth(800), winHeight(600),
    fullscreen(false),
    mouseX(0), mouseY(0),
    xRot(0.0f), yRot(0.0f)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(winX, winY);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Vil's OBJ Viewer");

    processArgs(argc, argv);

    // It's *really* important to create the window before you try to init Cg.
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
    glClear(GL_COLOR_BUFFER_BIT);

    float left = -0.5f;
    float right = 0.5f;
    float bottom = -0.5f;
    float top = 0.5f;

    float width = right - left;
    float height = top - bottom;

    glPushMatrix();
    //glRotatef(xRot, 0, 1, 0);
    //glRotatef(yRot, 1, 0, 0);

    //glutSolidCone(width / 4, height / 2, 360, 20);
    glutWireTeapot(fminf(width, height));

    glPopMatrix();
    glFlush();
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
    gluPerspective(30, double(winWidth) / double(winHeight), 0.001, 100.0);
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

    //xRot += x - mouseX;
    //yRot += y - mouseY;
    mouseY = y;
    mouseX = x;
    glutPostRedisplay();
}


void OBJViewerApp::run()
{
    glutMainLoop();
}


void OBJViewerApp::checkGLError(const char *errMsg, const char *okMsg)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        fprintf(stderr, "%s: %d\n", errMsg, err);
    } else if (okMsg != NULL) {
        fprintf(stderr, "%s\n", okMsg);
    }
}


void OBJViewerApp::init()
{
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


int main(int argc, char **argv)
{
    app = new OBJViewerApp(argc, argv);
    app->run();
    delete app;
    return 0;
}

