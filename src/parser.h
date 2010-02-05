#ifndef OBJViewer_parser_h
#define OBJViewer_parser_h

#include <stdexcept>
#include <string>

#include "model.h"


const unsigned int _MAX_LINE_LEN = 4096;


class ParseException : public virtual std::exception {
public:
  char message[_MAX_LINE_LEN];

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

  virtual void coordParsed(const Float4& coord) = 0;
  virtual void texCoordParsed(const Float4& coord) = 0;
  virtual void paramCoordParsed(const Float4& coord) = 0;
  virtual void normalParsed(const Float4& normal) = 0;
  virtual void faceParsed(Face* face) = 0;
  virtual void materialParsed(const std::string& name, Material* material) = 0;
  virtual void textureParsed(RawImage* texture) = 0;
};


void loadModel(ParserCallbacks* callbacks, const char* path) throw(ParseException);


#endif // OBJViewer_parser_h

