#ifndef OBJViewer_objviewer_h
#define OBJViewer_objviewer_h

#include <map>
#include "model.h"


//
// CLASSES
//

class OBJViewerApp {
public:
    OBJViewerApp(int argc, char **argv);
    ~OBJViewerApp();

    /**
     * Renders the scene. This will reload and rebind the vertex and fragment
     * shaders, as well as re-binding all the texture images.
     */
    void renderScene();

    /**
     * The callback function used by Glut whenever the window changes size.
     */
    void changeSize(int width, int height);

    /**
     * The callback function used by Glut whenever a key is pressed.
     */
    void keyPressed(unsigned char key, int x, int y);

    void mousePressed(int button, int state, int x, int y);
    void mouseDragged(int x, int y);

    /**
     * Starts the main event processing loop.
     */
    void run();

private:
    /**
     * Check for a GL error and report it if there was any. Can optionally report
     * the fact that there was no error as well.
     */
    void checkGLError(const char *errMsg = "GL error occurred", const char *okMsg = NULL);

    /**
     * Load and setup the shaders and images required to render the result.
     */
    void init();

    /**
     * Load an image file onto the GPU as a texture
     */
    void loadTexture(const char *path, int texID);

    /**
     * Prints help about the command line syntax and options to stderr.
     */
    void usage(char *progname);

    /**
     * Process the command line arguments.
     */
    void processArgs(int argc, char **argv);

    /**
     * Clears out the current shaders and textures.
     */
    void cleanUp();

private:
    int winX, winY, winWidth, winHeight;
    bool fullscreen;
    int mouseX, mouseY;
    float xRot, yRot;
    bool polygons;
    Model *model;
};


//
// FUNCTIONS
//



#endif // OBJViewer_objviewer_h

