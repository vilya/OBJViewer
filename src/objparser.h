#ifndef OBJViewer_objparser_h
#define OBJViewer_objparser_h

#include "parser.h"
#include "resources.h"


void loadOBJ(ParserCallbacks* callbacks, const char* path, ResourceManager* resources)
  throw(ParseException);


#endif // OBJViewer_objparser_h

