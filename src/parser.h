#ifndef OBJViewer_parser_h
#define OBJViewer_parser_h

#include <stdexcept>
#include <string>

//#include "math3d.h"
#include "vector.h"
#include "model.h"
#include "resources.h"


class ParseException : public virtual std::exception {
public:
  char message[4096];

  ParseException(const char *msg_format...);
  virtual ~ParseException() throw() {}

  virtual const char* what() const throw();
};


// Implement this and override the relevant methods to provide your own custom
// parsing behaviour.
class ParserCallbacks {
public:
  virtual void beginModel(const char* path) = 0;
  virtual void endModel() = 0;

  virtual void coordParsed(const vh::Vector3& coord) = 0;
  virtual void texCoordParsed(const vh::Vector2& coord) = 0;
  virtual void normalParsed(const vh::Vector3& normal) = 0;
  virtual void colorParsed(const vh::Vector4& color) = 0;
  virtual void faceParsed(Face* face) = 0;
  virtual void materialParsed(const std::string& name, Material* material) = 0;
  virtual void textureParsed(RawImage* texture) = 0;
};


void loadModel(ParserCallbacks* callbacks, const char* path, ResourceManager* resources)
  throw(ParseException);


#endif // OBJViewer_parser_h

