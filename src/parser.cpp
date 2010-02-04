#include <cstdarg>
#include <cstdio>
#include <libgen.h>

#include "parser.h"
#include "objparser.h"
#include "plyparser.h"

//
// EXCEPTION METHODS
//

ParseException::ParseException(const char *msg_format...) :
  std::exception()
{
  va_list args;
  va_start(args, msg_format);
  vsnprintf(message, _MAX_LINE_LEN, msg_format, args);
  va_end(args);
}


const char* ParseException::what() const throw()
{
  return message;
}


//
// PUBLIC FUNCTIONS
//

void loadModel(ParserCallbacks* callbacks, const char* path) throw(ParseException)
{
  if (callbacks == NULL)
    throw ParseException("You didn't provide any callbacks; parsing will do nothing!");

  const char *filename = basename(const_cast<char *>(path));
  if (filename == NULL)
    throw ParseException("Invalid model filename: %s does not name a file.", filename);

  // Check that the file exists.
  FILE *file = fopen(path, "rb");
  if (file == NULL)
    throw ParseException("File not found: %s.", path);
  else
    fclose(file);

  const char *ext = strrchr(filename, '.');
  if (ext == NULL)
    throw ParseException("Unknown model format.");

  if (strcasecmp(ext, ".obj") == 0) {
    callbacks->beginModel(path);
    loadOBJ(callbacks, path);
    callbacks->endModel();
  } else if (strcasecmp(ext, ".ply") == 0) {
    callbacks->beginModel(path);
    loadPLY(callbacks, path);
    callbacks->endModel();
  } else {
    throw ParseException("Unknown model format: %s", ext);
  }
}

