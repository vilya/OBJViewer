#ifndef OBJViewer_plyparser_h
#define OBJViewer_plyparser_h

#include "parser.h"
#include "resources.h"


void loadPLY(ParserCallbacks* callbacks, const char* path, ResourceManager* resources)
  throw(ParseException);


#endif // OBJViewer_plyparser_h

