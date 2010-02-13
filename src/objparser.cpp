#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <libgen.h>

#include <imagelib.h>
#include "model.h"
#include "parser.h"


//
// CONSTANTS
//

enum MTLFileLineType {
  MTL_LINETYPE_UNKNOWN,
  MTL_LINETYPE_BLANK,
  MTL_LINETYPE_COMMENT,
  MTL_LINETYPE_NEWMTL,
  MTL_LINETYPE_KA,
  MTL_LINETYPE_KD,
  MTL_LINETYPE_KE,
  MTL_LINETYPE_KM,
  MTL_LINETYPE_KS,
  MTL_LINETYPE_TF,
  MTL_LINETYPE_TR,
  MTL_LINETYPE_D,
  MTL_LINETYPE_NS,
  MTL_LINETYPE_NI,
  MTL_LINETYPE_ILLUM,
  MTL_LINETYPE_MAP_KA,
  MTL_LINETYPE_MAP_KD,
  MTL_LINETYPE_MAP_KE,
  MTL_LINETYPE_MAP_KM,
  MTL_LINETYPE_MAP_KS,
  MTL_LINETYPE_MAP_D,
  MTL_LINETYPE_MAP_BUMP,
  MTL_LINETYPE_BUMP 
};


enum OBJFileLineType {
  OBJ_LINETYPE_UNKNOWN,
  OBJ_LINETYPE_BLANK,
  OBJ_LINETYPE_COMMENT,
  OBJ_LINETYPE_V,
  OBJ_LINETYPE_VT,
  OBJ_LINETYPE_VP,
  OBJ_LINETYPE_VN,
  OBJ_LINETYPE_F,
  OBJ_LINETYPE_FO,
  OBJ_LINETYPE_G,
  OBJ_LINETYPE_S,
  OBJ_LINETYPE_USEMTL,
  OBJ_LINETYPE_MTLLIB,
  OBJ_LINETYPE_O
};


//
// GLOBAL VARIABLES
//

std::map<std::string, RawImage*> gTextures;


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
  while (*col== '.' || *col == '_' || *col == '-' || *col == '(' || *col == ')' ||
      isLetter(*col) || isDigit(*col))
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
    }
    ++col;
  }

  // Trim off trailing whitespace.
  while (col > line && isSpace(*(col - 1)))
    --col;

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
  static const struct {
    const char* token;
    MTLFileLineType lineType;
  } LINE_TYPES[] = {
    { "newmtl", MTL_LINETYPE_NEWMTL },
    { "Ka", MTL_LINETYPE_KA },
    { "Kd", MTL_LINETYPE_KD },
    { "Ke", MTL_LINETYPE_KE },
    { "Km", MTL_LINETYPE_KM },
    { "Ks", MTL_LINETYPE_KS },
    { "Tf", MTL_LINETYPE_TF },
    { "Tr", MTL_LINETYPE_TR },
    { "d", MTL_LINETYPE_D },
    { "Ns", MTL_LINETYPE_NS },
    { "Ni", MTL_LINETYPE_NI },
    { "illum", MTL_LINETYPE_ILLUM },
    { "map_Ka", MTL_LINETYPE_MAP_KA },
    { "map_Kd", MTL_LINETYPE_MAP_KD },
    { "map_Ke", MTL_LINETYPE_MAP_KE },
    { "map_Km", MTL_LINETYPE_MAP_KM },
    { "map_Ks", MTL_LINETYPE_MAP_KS },
    { "map_D", MTL_LINETYPE_MAP_D },
    { "map_Bump", MTL_LINETYPE_MAP_BUMP },
    { "bump", MTL_LINETYPE_MAP_BUMP },
    { "#", MTL_LINETYPE_COMMENT },
    { NULL, MTL_LINETYPE_UNKNOWN }
  };

  col = line;
  while (*col == '_' || isLetter(*col) || isDigit(*col))
    ++col;

  int len = col - line;
  if (len == 0) {
    return MTL_LINETYPE_BLANK;
  } else {
    for (unsigned int i = 0; LINE_TYPES[i].token != NULL; ++i) {
      if (strncasecmp(LINE_TYPES[i].token, line, len) == 0)
        return LINE_TYPES[i].lineType;
    }
    return MTL_LINETYPE_UNKNOWN;
  }
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


float mtlParseFloat(char *line, char*& col) throw(ParseException)
{
  col = line;
  eatSpace(col, true);
  float val = parseFloat(col, col);
  return val;
}


RawImage* mtlParseTexture(char* line, char*& col, const char* baseDir) throw(ParseException)
{
  col = line;
  eatSpace(col, true);

  std::string filename = resolvePath(baseDir, parseFilename(col, col));
  std::map<std::string, RawImage*>::const_iterator texIter = gTextures.find(filename);
  if (texIter != gTextures.end())
    return texIter->second;

  fprintf(stderr, "Loading texture %s...", filename.c_str());

  RawImage* tex = NULL;
  try {
    tex = new RawImage(filename.c_str());
    gTextures[filename] = tex;
  } catch (ImageException& ex) {
    throw ParseException("Error loading texture map: %s", ex.what());
  }

  if (tex != NULL)
    fprintf(stderr, " %dx%d pixels.\n", tex->getWidth(), tex->getHeight());
  return tex;
}


void loadMaterialLibrary(ParserCallbacks* callbacks, const char* path,
    std::map<std::string, Material*>& materials)
  throw(ParseException)
{
  fprintf(stderr, "Loading mtllib %s...\n", path);

  FILE *f = fopen(path, "r");
  if (f == NULL)
    throw ParseException("Unable to open mtl file %s: %s\n", path, strerror(errno));
  
  char line[_MAX_LINE_LEN];
  char *col;
  unsigned int line_no = 0;

  char baseDir[_MAX_LINE_LEN];
  snprintf(baseDir, _MAX_LINE_LEN, "%s", dirname(const_cast<char*>(path)));

  std::string materialName;
  Material *material = NULL;
  try {
    while (!feof(f)) {
      ++line_no;

      memset(line, 0, _MAX_LINE_LEN);
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
            materials[materialName] = material;
            callbacks->materialParsed(materialName, material);
            material = NULL;
          }
          materialName = mtlParseNEWMTL(col, col);
          if (materials.count(materialName) > 0)
            throw ParseException("Redefinition of material %s", materialName.c_str());
          material = new Material();
          break;
        case MTL_LINETYPE_KA:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->Ka = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_KD:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->Kd = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_KS:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->Ks = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_TF:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->Tf = mtlParseColor(col, col);
          break;
        case MTL_LINETYPE_D:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->d = mtlParseFloat(col, col);
          break;
        case MTL_LINETYPE_NS:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->Ns = mtlParseFloat(col, col);
          break;
        case MTL_LINETYPE_MAP_KA:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->mapKa = mtlParseTexture(col, col, baseDir);
          callbacks->textureParsed(material->mapKa);
          break;
        case MTL_LINETYPE_MAP_KD:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->mapKd = mtlParseTexture(col, col, baseDir);
          callbacks->textureParsed(material->mapKd);
          break;
        case MTL_LINETYPE_MAP_KS:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->mapKs = mtlParseTexture(col, col, baseDir);
          callbacks->textureParsed(material->mapKs);
          break;
        case MTL_LINETYPE_MAP_D:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->mapD = mtlParseTexture(col, col, baseDir);
          callbacks->textureParsed(material->mapD);
          break;
        case MTL_LINETYPE_MAP_BUMP:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          material->mapBump = mtlParseTexture(col, col, baseDir);
          callbacks->textureParsed(material->mapBump);
          break;
        case MTL_LINETYPE_KE:
        case MTL_LINETYPE_KM:
        case MTL_LINETYPE_MAP_KE:
        case MTL_LINETYPE_MAP_KM:
        case MTL_LINETYPE_NI:
        case MTL_LINETYPE_ILLUM:
        case MTL_LINETYPE_TR:
        case MTL_LINETYPE_BUMP:
          if (material == NULL)
            throw ParseException("Defining a material property without declaring a material name.");
          // TODO: handle these.
          while (!isEnd(*col))
            ++col;
          break;
        case MTL_LINETYPE_BLANK:
        case MTL_LINETYPE_COMMENT:
          // Ignore these types of line.
          break;
        default:
          throw ParseException("Unknown line type: %s", line);
      }

      eatSpace(col);
      if (!isCommentStart(*col) && !isEnd(*col))
        throw ParseException("Unexpected trailing characters: %s", col);
    }

    if (material != NULL) {
      materials[materialName] = material;
      callbacks->materialParsed(materialName, material);
      material = NULL;
    }
    fprintf(stderr, "Finished parsing mtllib %s\n", path);
  } catch (ParseException& ex) {
    fclose(f);
    if (material != NULL)
      delete material;
    throw ParseException("[%s: line %d, col %d] %s\n", path, line_no, (int)(col - line), ex.message);
  }
}



//
// OBJ FILE PARSING
//

OBJFileLineType objParseLineType(char* line, char*& col) throw(ParseException) {
  static const struct {
    const char* token;
    OBJFileLineType lineType;
  } LINE_TYPES[] = {
    { "#", OBJ_LINETYPE_COMMENT },
    { "v", OBJ_LINETYPE_V },
    { "vt", OBJ_LINETYPE_VT },
    { "vp", OBJ_LINETYPE_VP },
    { "vn", OBJ_LINETYPE_VN },
    { "f", OBJ_LINETYPE_F },
    { "fo", OBJ_LINETYPE_FO },
    { "g", OBJ_LINETYPE_G },
    { "s", OBJ_LINETYPE_S },
    { "usemtl", OBJ_LINETYPE_USEMTL },
    { "mtllib", OBJ_LINETYPE_MTLLIB },
    { "o", OBJ_LINETYPE_O },
    { NULL, OBJ_LINETYPE_UNKNOWN }
  };

  col = line;
  while (isLetter(*col) || isDigit(*col))
    ++col;

  int len = col - line;
  if (len == 0) {
    return OBJ_LINETYPE_BLANK;
  } else {
    for (unsigned int i = 0; LINE_TYPES[i].token != NULL; ++i) {
      if (strncmp(LINE_TYPES[i].token, line, len) == 0)
        return LINE_TYPES[i].lineType;
    }
    return OBJ_LINETYPE_UNKNOWN;
  };
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
    if (*col == '-' || isDigit(*col))
      vt = parseInt(col, col) - 1;
    if (*col == '/') {
      eatChar('/', col);
      if (*col == '-' || isDigit(*col))
        vn = parseInt(col, col) - 1;
    }
  }
  return Vertex(v, vt, vn, -1);
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


void objParseMTLLIB(char* line, char*& col,
    ParserCallbacks* callbacks, const char* baseDir,
    std::map<std::string, Material*>& materials)
  throw(ParseException)
{
  col = line;
  while (!isEnd(*col) && !isCommentStart(*col)) {
    eatSpace(col, true);
    if (!isEnd(*col) && !isCommentStart(*col)) {
      std::string filename = resolvePath(baseDir, parseFilename(col, col));
      loadMaterialLibrary(callbacks, filename.c_str(), materials);
    }
  }
}


Material *objParseUSEMTL(char *line, char*& col, std::map<std::string, Material*>& materials)
  throw(ParseException)
{
  col = line;
  eatSpace(col, true);
  std::string name = parseIdentifier(col, col);
  Material *material = materials[name];
  return material;
}


//
// PUBLIC FUNCTIONS
//

void loadOBJ(ParserCallbacks* callbacks, const char* path) throw(ParseException)
{
  FILE *f = fopen(path, "r");
  if (f == NULL)
    throw ParseException("Unable to open file %s.\n", path);
  
  char line[_MAX_LINE_LEN];
  char *col;
  unsigned int line_no = 0;

  std::map<std::string, Material*> materials;
  Material *activeMaterial = NULL;
  try {
    while (!feof(f)) {
      ++line_no;

      if (fgets(line, _MAX_LINE_LEN, f) == NULL) {
        if (ferror(f))
          throw ParseException("Error reading from file %s", strerror(ferror(f)));
        else
          break;
      }

      col = line;
      eatSpace(col);
      switch (objParseLineType(col, col)) {
        case OBJ_LINETYPE_V:
          callbacks->coordParsed(objParseV(col, col));
          break;
        case OBJ_LINETYPE_VT:
          callbacks->texCoordParsed(objParseVT(col, col));
          break;
        case OBJ_LINETYPE_VN:
          callbacks->normalParsed(objParseVN(col, col));
          break;
        case OBJ_LINETYPE_F:
        case OBJ_LINETYPE_FO:
          callbacks->faceParsed(objParseFace(col, col, activeMaterial));
          // TODO: check for -ve indexes and resolve them to +ve ones.
          break;
        case OBJ_LINETYPE_USEMTL:
          activeMaterial = objParseUSEMTL(col, col, materials);
          break;
        case OBJ_LINETYPE_MTLLIB:
          objParseMTLLIB(col, col, callbacks, dirname(const_cast<char*>(path)), materials);
          break;
        case OBJ_LINETYPE_VP:
        case OBJ_LINETYPE_G:
        case OBJ_LINETYPE_S:
        case OBJ_LINETYPE_O:
          // TODO: handle this.
          while (!isEnd(*col))
            ++col;
          break;
        case OBJ_LINETYPE_BLANK:
        case OBJ_LINETYPE_COMMENT:
          // Ignore these types of lines.
          break;
        default:
          throw ParseException("Unknown line type %s", line);
      }

      eatSpace(col);
      if (!isCommentStart(*col) && !isEnd(*col))
        throw ParseException("Unexpected trailing characters: %s", col);
    }
  } catch (ParseException& ex) {
    fclose(f);
    throw ParseException("[%s: line %d, col %d] %s\n", path, line_no, (int)(col - line), ex.what());
  }
}


