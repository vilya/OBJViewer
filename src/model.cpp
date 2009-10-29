#include <cstdarg>
#include <cstdio>
#include <libgen.h>

#include "model.h"


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



//
// CONSTANTS
//

enum MTLFileLineType {
  MTL_LINETYPE_UNKNOWN,
  MTL_LINETYPE_BLANK,
  MTL_LINETYPE_NEWMTL,
  MTL_LINETYPE_KA,
  MTL_LINETYPE_KD,
  MTL_LINETYPE_KS,
  MTL_LINETYPE_TF,
  MTL_LINETYPE_D,
  MTL_LINETYPE_NS,
  MTL_LINETYPE_NI,
  MTL_LINETYPE_ILLUM,
  MTL_LINETYPE_MAP_KD,
  MTL_LINETYPE_COMMENT
};


enum OBJFileLineType {
  OBJ_LINETYPE_UNKNOWN,
  OBJ_LINETYPE_BLANK,
  OBJ_LINETYPE_V,
  OBJ_LINETYPE_VT,
  OBJ_LINETYPE_VP,
  OBJ_LINETYPE_VN,
  OBJ_LINETYPE_F,
  OBJ_LINETYPE_G,
  OBJ_LINETYPE_S,
  OBJ_LINETYPE_USEMTL,
  OBJ_LINETYPE_MTLLIB,
  OBJ_LINETYPE_COMMENT
};


//
// FUNCTIONS
//

bool isSpace(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}


bool isDigit(char ch) {
  return (ch >= '0' && ch <= '9');
}


bool isLetter(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


bool isEnd(char ch) {
  return (ch == '\0');
}


bool isCommentStart(char ch) {
  return (ch == '#');
}


void eatSpace(char*& col, bool required=false) throw(ParseException) {
  if (required && !isSpace(*col))
    throw ParseException("Expected whitespace but got %s", col);
  while (isSpace(*col))
    ++col;
}


void eatChar(char ch, char*& col) throw(ParseException) {
  if (*col != ch)
    throw ParseException("Expected %c but got %c", ch, *col);
  ++col;
}


std::string resolvePath(const std::string& baseDir, const std::string& path) throw(ParseException) {
  if (baseDir.size() == 0 || path[0] == '/') {
    return path;
  } else {
    int len = baseDir.size();
    if (len == 0 || baseDir[len-1] == '/')
      return baseDir + path;
    else
      return baseDir + "/" + path;
  }
}


//
// GENERIC PARSING
//

float parseFloat(char *line, char*& col) throw(ParseException) {
  col = line;
  if (*col == '-' || *col == '+')
    ++col;
  while (isDigit(*col))
    ++col;
  if (*col == '.') {
    ++col;
    while (isDigit(*col))
      ++col;
  }
  if (*col == 'e' || *col == 'E') {
    ++col;
    if (*col == '+' || *col == '-')
      ++col;
    while (isDigit(*col))
      ++col;
  }
  if (col > line) {
    float val;
    if (sscanf(line, "%f", &val) == 1)
      return val;
  }
  throw ParseException("Expected a float value but got %s", line);
}


int parseInt(char* line, char*& col) throw(ParseException) {
  col = line;
  if (*col == '-' || *col == '+')
    ++col;
  while (isDigit(*col))
    ++col;

  if (col > line) {
    int val;
    if (sscanf(line, "%d", &val) == 1)
      return val;
  }
  throw ParseException("Expected an int value but got \"%s\"", line);
}


std::string parseIdentifier(char* line, char*& col) throw(ParseException) {
  col = line;
  while (*col == '_' || isLetter(*col) || isDigit(*col))
    ++col;

  if (col > line) {
    char buf[_MAX_LINE_LEN];
    snprintf(buf, (int)(col - line) + 1, "%s", line);
    return std::string(buf);
  }
  throw ParseException("Expected an identifier but got \"%s\"", line);
}


std::string parseFilename(char* line, char*& col) throw(ParseException) {
  col = line;
  char quote = '\0';
  while (!isEnd(*col) && !isCommentStart(*col)) {
    if (*col == '\\') {
      ++col;
    } else if (*col == '"' || *col == '\'') {
      if (!quote)
        quote = *col;
      else if (quote == *col)
        quote = '\0';
    } else if (isSpace(*col) && !quote) {
      break;
    }
    ++col;
  }

  if (quote)
    throw ParseException("Unclosed filename string: missing closing %c character", quote);

  char buf[_MAX_LINE_LEN];
  snprintf(buf, (int)(col - line) + 1, "%s", line);
  return std::string(buf);
}


//
// MTL FILE PARSING
//

MTLFileLineType mtlParseLineType(char* line, char*& col) throw(ParseException) {
  col = line;
  while (isLetter(*col) || isDigit(*col))
    ++col;

  int len = col - line;
  if (len == 0)
    return MTL_LINETYPE_BLANK;
  else if (strncmp("newmtl", line, len) == 0)
    return MTL_LINETYPE_NEWMTL;
  else if (strncmp("Ka", line, len) == 0)
    return MTL_LINETYPE_KA;
  else if (strncmp("Kd", line, len) == 0)
    return MTL_LINETYPE_KD;
  else if (strncmp("Ks", line, len) == 0)
    return MTL_LINETYPE_KS;
  else if (strncmp("Tf", line, len) == 0)
    return MTL_LINETYPE_TF;
  else if (strncmp("d", line, len) == 0)
    return MTL_LINETYPE_D;
  else if (strncmp("Ns", line, len) == 0)
    return MTL_LINETYPE_NS;
  else if (strncmp("Ni", line, len) == 0)
    return MTL_LINETYPE_NI;
  else if (strncmp("illum", line, len) == 0)
    return MTL_LINETYPE_ILLUM;
  else if (strncmp("map_Kd", line, len) == 0)
    return MTL_LINETYPE_MAP_KD;
  else if (strncmp("#", line, len) == 0)
    return MTL_LINETYPE_COMMENT;
  else
    return MTL_LINETYPE_UNKNOWN;
}


std::string mtlParseNEWMTL(char* line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  std::string name = parseIdentifier(col, col);
  return name;
}


Float4 mtlParseColor(char* line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float r = parseFloat(col, col);
  eatSpace(col, false);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(r, r, r);
  float g = parseFloat(col, col);
  eatSpace(col, true);
  float b = parseFloat(col, col);
  return Float4(r, g, b);
}


float mtlParseFloat(char *line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float val = parseFloat(col, col);
  return val;
}


void loadMaterialLibrary(const char* path, std::map<std::string, Material>& materials) throw(ParseException) {
  FILE *f = fopen(path, "r");
  if (f == NULL)
    throw ParseException("Unable to open mtl file %s.\n", path);
  
  char line[_MAX_LINE_LEN];
  char *col;
  unsigned int line_no = 0;

  std::string materialName;
  Material *material = NULL;
  try {
    while (!feof(f)) {
      ++line_no;

      if (fgets(line, _MAX_LINE_LEN, f) == NULL) {
        if (ferror(f))
          throw ParseException("Error reading from file: %s", strerror(ferror(f)));
        else
          break;
      }

      col = line;
      eatSpace(col);
      switch (mtlParseLineType(col, col)) {
        case MTL_LINETYPE_NEWMTL:
          if (material != NULL) {
            materials[materialName] = *material;
            delete material;
            material = NULL;
          }
          materialName = mtlParseNEWMTL(col, col);
          if (materials.count(materialName) == 0)
            material = new Material();
          break;
        case MTL_LINETYPE_KA:
          if (material != NULL)
            material->Ka = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_KD:
          if (material != NULL)
            material->Kd = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_KS:
          if (material != NULL)
            material->Ks = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_TF:
          if (material != NULL)
            material->Tf = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_D:
          if (material != NULL)
            material->d = mtlParseFloat(col, col);
          break;
        case MTL_LINETYPE_NS:
          if (material != NULL)
            material->Ns = mtlParseFloat(col, col);
          break;
        case MTL_LINETYPE_NI:
        case MTL_LINETYPE_ILLUM:
        case MTL_LINETYPE_MAP_KD:
          // TODO: handle these.
          while (!isEnd(*col))
            ++col;
          break;
        case MTL_LINETYPE_BLANK:
        case MTL_LINETYPE_COMMENT:
          break;
        default:
          throw ParseException("Unknown line type: %s", line);
      }

      eatSpace(col);
      if (!isCommentStart(*col) && !isEnd(*col))
        throw ParseException("Unexpected trailing characters: %s", col);
    }

    if (material != NULL) {
      materials[materialName] = *material;
      delete material;
      material = NULL;
    }
  } catch (ParseException& ex) {
    fclose(f);
    if (material != NULL)
      delete material;
    fprintf(stderr, "[%s: line %d, col %d] %s\n", path, line_no, (int)(col - line), ex.message);
    throw ex;
  }
}



//
// OBJ FILE PARSING
//

OBJFileLineType objParseLineType(char* line, char*& col) throw(ParseException) {
  col = line;
  while (isLetter(*col) || isDigit(*col))
    ++col;

  int len = col - line;
  if (len == 0)
    return OBJ_LINETYPE_BLANK;
  else if (strncmp("v", line, len) == 0)
    return OBJ_LINETYPE_V;
  else if (strncmp("vt", line, len) == 0)
    return OBJ_LINETYPE_VT;
  else if (strncmp("vp", line, len) == 0)
    return OBJ_LINETYPE_VP;
  else if (strncmp("vn", line, len) == 0)
    return OBJ_LINETYPE_VN;
  else if (strncmp("f", line, len) == 0)
    return OBJ_LINETYPE_F;
  else if (strncmp("g", line, len) == 0)
    return OBJ_LINETYPE_G;
  else if (strncmp("s", line, len) == 0)
    return OBJ_LINETYPE_S;
  else if (strncmp("usemtl", line, len) == 0)
    return OBJ_LINETYPE_USEMTL;
  else if (strncmp("mtllib", line, len) == 0)
    return OBJ_LINETYPE_MTLLIB;
  else if (strncmp("#", line, len) == 0)
    return OBJ_LINETYPE_COMMENT;
  else
    return OBJ_LINETYPE_UNKNOWN;
}


Float4 objParseV(char *line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float x = parseFloat(col, col);
  eatSpace(col, true);
  float y = parseFloat(col, col);
  eatSpace(col, true);
  float z = parseFloat(col, col);
  eatSpace(col);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(x, y, z);
  float w = parseFloat(col, col);
  return Float4(x, y, z, w);
}


Float4 objParseVT(char *line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float u = parseFloat(col, col);
  eatSpace(col);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(u, 0.0f, 0.0f);
  float v = parseFloat(col, col);
  eatSpace(col);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(u, v, 0.0f);
  float w = parseFloat(col, col);
  return Float4(u, v, w);
}


Float4 objParseVP(char *line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float u = parseFloat(col, col);
  eatSpace(col);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(u, 0.0f, 0.0f);
  float v = parseFloat(col, col);
  eatSpace(col);
  if (isEnd(*col) || isCommentStart(*col))
    return Float4(u, v, 0.0f);
  float w = parseFloat(col, col);
  return Float4(u, v, w);
}


Float4 objParseVN(char* line, char*& col) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  float i = parseFloat(col, col);
  eatSpace(col, true);
  float j = parseFloat(col, col);
  eatSpace(col, true);
  float k = parseFloat(col, col);
  return Float4(i, j, k);
}


Vertex objParseVertex(char *line, char*& col) throw(ParseException) {
  col = line;
  int v = parseInt(col, col) - 1;
  int vt = -1;
  int vn = -1;
  if (*col == '/') {
    eatChar('/', col);
    vt = (*col != '/') ? parseInt(col, col) - 1 : -1;
    if (*col == '/') {
      eatChar('/', col);
      vn = parseInt(col, col) - 1;
    }
  }
  return Vertex(v, vt, vn);
}


Face *objParseFace(char* line, char*& col, Material *activeMaterial) throw(ParseException) {
  Face *face = new Face(activeMaterial);
  try {
    col = line;
    while (!isEnd(*col) && !isCommentStart(*col)) {
      eatSpace(col, true);
      if (!isEnd(*col) && !isCommentStart(*col))
        face->vertexes.push_back(objParseVertex(col, col));
    }
  } catch (ParseException& ex) {
    delete face;
    throw ex;
  }
  return face;
}


void objParseMTLLIB(char* line, char*& col, const char* baseDir, std::map<std::string, Material>& materials) throw(ParseException) {
  col = line;
  while (!isEnd(*col) && !isCommentStart(*col)) {
    eatSpace(col, true);
    if (!isEnd(*col) && !isCommentStart(*col)) {
      std::string filename = resolvePath(baseDir, parseFilename(col, col));
      loadMaterialLibrary(filename.c_str(), materials);
    }

  }
}


Material *objParseUSEMTL(char *line, char*& col, std::map<std::string, Material>& materials) throw(ParseException) {
  col = line;
  eatSpace(col, true);
  std::string name = parseIdentifier(col, col);
  Material *material = &materials[name];
  return material;
}


Model* loadModel(const char* path) throw(ParseException) {
  FILE *f = fopen(path, "r");
  if (f == NULL)
    throw ParseException("Unable to open model file %s.\n", path);
  
  char line[_MAX_LINE_LEN];
  char *col;
  unsigned int line_no = 0;

  Model *model = new Model();
  Material *activeMaterial = NULL;
  try {
    while (!feof(f)) {
      ++line_no;

      if (fgets(line, _MAX_LINE_LEN, f) == NULL) {
        if (ferror(f))
          throw ParseException("Error reading from file: %s", strerror(ferror(f)));
        else
          break;
      }

      col = line;
      eatSpace(col);
      switch (objParseLineType(col, col)) {
        case OBJ_LINETYPE_V:
          model->v.push_back(objParseV(col, col));
          break;
        case OBJ_LINETYPE_VT:
          model->vt.push_back(objParseVT(col, col));
          break;
        case OBJ_LINETYPE_VP:
          model->vp.push_back(objParseVP(col, col));
          break;
        case OBJ_LINETYPE_VN:
          model->vn.push_back(objParseVN(col, col));
          break;
        case OBJ_LINETYPE_F:
          model->faces.push_back(objParseFace(col, col, activeMaterial));
          break;
        case OBJ_LINETYPE_G:
          // TODO: handle this.
          while (!isEnd(*col))
            ++col;
          break;
        case OBJ_LINETYPE_S:
          // TODO: handle this.
          while (!isEnd(*col))
            ++col;
          break;
        case OBJ_LINETYPE_USEMTL:
          activeMaterial = objParseUSEMTL(col, col, model->materials);
          break;
        case OBJ_LINETYPE_MTLLIB:
          objParseMTLLIB(col, col, dirname(const_cast<char*>(path)), model->materials);
          break;
        case OBJ_LINETYPE_BLANK:
        case OBJ_LINETYPE_COMMENT:
          break;
        default:
          throw ParseException("Unknown line type: %s", line);
      }

      eatSpace(col);
      if (!isCommentStart(*col) && !isEnd(*col))
        throw ParseException("Unexpected trailing characters: %s", col);
    }
  } catch (ParseException& ex) {
    fclose(f);
    if (model != NULL)
      delete model;
    fprintf(stderr, "[%s: line %d, col %d] %s\n", path, line_no, (int)(col - line), ex.message);
    throw ex;
  }
  return model;
}
