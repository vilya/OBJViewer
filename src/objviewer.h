#ifndef OBJViewer_objviewer_h
#define OBJViewer_objviewer_h

#include <map>
#include "image.h"
#include "model.h"
#include "renderer.h"


//
// CLASSES
//

class OBJViewerApp {
public:
    OBJViewerApp(int argc, char **argv);
    ~OBJViewerApp();

    void redraw();
    void changeSize(int width, int height);
    void keyPressed(unsigned char key, int x, int y);
    void mousePressed(int button, int state, int x, int y);
    void mouseDragged(int x, int y);
    void run();

private:
    //! Prints help about the command line syntax and options to stderr.
    void usage(char *progname);

    //! Process the command line arguments.
    void processArgs(int argc, char **argv);

private:
    int winX, winY, winWidth, winHeight, currWidth, currHeight;
    bool fullscreen;
    int mouseX, mouseY, mouseModifiers;
    Renderer* _renderer;
    Model *_model;
};


#endif // OBJViewer_objviewer_h

