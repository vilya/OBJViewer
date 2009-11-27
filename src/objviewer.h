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

    //! Renders the scene. This will reload and rebind the vertex and fragment
    //! shaders, as well as re-binding all the texture images.
    void renderScene();

    //! The callback function used by Glut whenever the window changes size.
    void changeSize(int width, int height);

    //! The callback function used by Glut whenever a key is pressed.
    void keyPressed(unsigned char key, int x, int y);

    void mousePressed(int button, int state, int x, int y);
    void mouseDragged(int x, int y);

    //! Starts the main event processing loop.
    void run();

private:
    //! Compile the display lists for the model.
    void setupDisplayLists();

    //! Load and setup the shaders and images required to render the result.
    void init();

    //! Load all textures for the model onto the GPU.
    void loadTexturesForModel(Model* theModel);

    //! Draw the specified model as either filled polygons or lines.
    void drawModel(Model* theModel, bool filledPolygons);

    //! Load an image file onto the GPU as a texture
    void loadTexture(Image& tex, int texID);

    //! Prints help about the command line syntax and options to stderr.
    void usage(char *progname);

    //! Process the command line arguments.
    void processArgs(int argc, char **argv);

    //! Clears out the current shaders and textures.
    void cleanUp();

private:
    int winX, winY, winWidth, winHeight;
    bool fullscreen;
    int mouseX, mouseY;
    float xRot, yRot;
    bool polygons;
    Model *model;
    unsigned int modelDisplayList, linesDisplayList;
};


#endif // OBJViewer_objviewer_h

