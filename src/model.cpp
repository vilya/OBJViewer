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

enum LineType {
    LINETYPE_UNKNOWN,
    LINETYPE_BLANK,
    LINETYPE_V,
    LINETYPE_VT,
    LINETYPE_VP,
    LINETYPE_VN,
    LINETYPE_F,
    LINETYPE_G,
    LINETYPE_USEMTL,
    LINETYPE_MTLLIB,
    LINETYPE_COMMENT
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


LineType parseLineType(char* line, char*& col) throw(ParseException) {
    col = line;
    while (isLetter(*col) || isDigit(*col))
        ++col;

    int len = col - line;
    if (len == 0)
        return LINETYPE_BLANK;
    else if (strncmp("v", line, len) == 0)
        return LINETYPE_V;
    else if (strncmp("vt", line, len) == 0)
        return LINETYPE_VT;
    else if (strncmp("vp", line, len) == 0)
        return LINETYPE_VP;
    else if (strncmp("vn", line, len) == 0)
        return LINETYPE_VN;
    else if (strncmp("f", line, len) == 0)
        return LINETYPE_F;
    else if (strncmp("g", line, len) == 0)
        return LINETYPE_G;
    else if (strncmp("usemtl", line, len) == 0)
        return LINETYPE_USEMTL;
    else if (strncmp("mtllib", line, len) == 0)
        return LINETYPE_MTLLIB;
    else if (strncmp("#", line, len) == 0)
        return LINETYPE_COMMENT;
    else
        return LINETYPE_UNKNOWN;
}


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


Vec4 parseV(char *line, char*& col) throw(ParseException) {
    col = line;
    eatSpace(col, true);
    float x = parseFloat(col, col);
    eatSpace(col, true);
    float y = parseFloat(col, col);
    eatSpace(col, true);
    float z = parseFloat(col, col);
    eatSpace(col);
    if (isEnd(*col))
        return Vec4(x, y, z);
    float w = parseFloat(col, col);
    return Vec4(x, y, z, w);
}


Vec4 parseVT(char *line, char*& col) throw(ParseException) {
    col = line;
    eatSpace(col, true);
    float u = parseFloat(col, col);
    eatSpace(col);
    if (isEnd(*col))
        return Vec4(u, 0.0f, 0.0f);
    float v = parseFloat(col, col);
    eatSpace(col);
    if (isEnd(*col))
        return Vec4(u, v, 0.0f);
    float w = parseFloat(col, col);
    return Vec4(u, v, w);
}


Vec4 parseVP(char *line, char*& col) throw(ParseException) {
    col = line;
    eatSpace(col, true);
    float u = parseFloat(col, col);
    eatSpace(col);
    if (isEnd(*col))
        return Vec4(u, 0.0f, 0.0f);
    float v = parseFloat(col, col);
    eatSpace(col);
    if (isEnd(*col))
        return Vec4(u, v, 0.0f);
    float w = parseFloat(col, col);
    return Vec4(u, v, w);
}


Vec4 parseVN(char* line, char*& col) throw(ParseException) {
    col = line;
    eatSpace(col, true);
    float i = parseFloat(col, col);
    eatSpace(col, true);
    float j = parseFloat(col, col);
    eatSpace(col, true);
    float k = parseFloat(col, col);
    return Vec4(i, j, k);
}


Vertex parseVertex(char *line, char*& col) throw(ParseException) {
    col = line;
    int v = parseInt(col, col) - 1;
    eatChar('/', col);
    int vt = (*col != '/') ? parseInt(col, col) - 1 : -1;
    eatChar('/', col);
    int vn = parseInt(col, col) - 1;
    return Vertex(v, vt, vn);
}


Face *parseFace(char* line, char*& col) throw(ParseException) {
    Face *face = new Face();
    try {
        col = line;
        while (!isEnd(*col) && !isCommentStart(*col)) {
            eatSpace(col, true);
            if (!isEnd(*col) && !isCommentStart(*col))
                face->vertexes.push_back(parseVertex(col, col));
        }
    } catch (ParseException& ex) {
        delete face;
        throw ex;
    }
    return face;
}


Model* loadModel(const char* path) throw(ParseException) {
    FILE *f = fopen(path, "r");
    if (f == NULL)
        throw ParseException("Unable to open model file %s.\n", path);
    
    char line[_MAX_LINE_LEN];
    char *col;
    unsigned int line_no = 0;

    Model *model = new Model();
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
            switch (parseLineType(col, col)) {
                case LINETYPE_V:
                    model->v.push_back(parseV(col, col));
                    break;
                case LINETYPE_VT:
                    model->vt.push_back(parseVT(col, col));
                    break;
                case LINETYPE_VP:
                    model->vp.push_back(parseVP(col, col));
                    break;
                case LINETYPE_VN:
                    model->vn.push_back(parseVN(col, col));
                    break;
                case LINETYPE_F:
                    model->faces.push_back(parseFace(col, col));
                    break;
                case LINETYPE_G:
                case LINETYPE_USEMTL:
                case LINETYPE_MTLLIB:
                    // TODO: handle these.
                    while (!isEnd(*col))
                        ++col;
                    break;
                case LINETYPE_BLANK:
                case LINETYPE_COMMENT:
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
