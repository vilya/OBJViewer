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


Model* loadModel(const char* path,
    unsigned int startFrame, unsigned int endFrame) throw(ParseException);


#endif // OBJViewer_parser_h

