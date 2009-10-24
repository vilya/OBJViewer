#include <cstdarg>
#include <cstdio>
#include <libgen.h>

#include "model.h"


//
// EXCEPTION METHODS
//

AnyException::AnyException(const char *msg_format...) :
	std::exception()
{
	va_list args;
	va_start(args, msg_format);
	vsnprintf(message, _MAX_LINE_LEN, msg_format, args);
	va_end(args);
}


ParseException::ParseException(const char *a_line, unsigned int a_line_number, unsigned int a_col_number, const char *msg_format...) :
	std::exception(), line_number(a_line_number), col_number(a_col_number)
{
	strncpy(line, a_line, _MAX_LINE_LEN);
	va_list args;
	va_start(args, msg_format);
	vsnprintf(message, _MAX_LINE_LEN, msg_format, args);
	va_end(args);
}



//
// FUNCTIONS
//

enum LineType {
    LINETYPE_UNKNOWN,
    LINETYPE_BLANK,
    LINETYPE_V,
    LINETYPE_VT,
    LINETYPE_VP,
    LINETYPE_VN,
    LINETYPE_F,
    LINETYPE_USEMTL,
    LINETYPE_MTLLIB,
    LINETYPE_COMMENT
};


bool isSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}


bool isDigit(char ch) {
    return (ch >= '0' && ch <= '9');
}


bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


LineType classify(char* line) {
    char *col = line;
    bool isWordChar = false;

    while (isSpace(*col))
        ++col;

    char *start = col;
    while (isLetter(*col) || isDigit(*col))
        ++col;

    int len = col - start;
    if (len == 0)
        return LINETYPE_BLANK;
    else if (strncmp("v", start, len) == 0)
        return LINETYPE_V;
    else if (strncmp("vt", start, len) == 0)
        return LINETYPE_VT;
    else if (strncmp("vp", start, len) == 0)
        return LINETYPE_VP;
    else if (strncmp("vn", start, len) == 0)
        return LINETYPE_VN;
    else if (strncmp("f", start, len) == 0)
        return LINETYPE_F;
    else if (strncmp("usemtl", start, len) == 0)
        return LINETYPE_USEMTL;
    else if (strncmp("mtllib", start, len) == 0)
        return LINETYPE_MTLLIB;
    else if (strncmp("#", start, len) == 0)
        return LINETYPE_COMMENT;
    else
        return LINETYPE_UNKNOWN;
}

Model* loadModel(const char* path) throw(ModelException)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        throw ModelException("Unable to open model file %s.\n", path);
    
    Model *model = new Model();
    try {
        char line[_MAX_LINE_LEN];
        char *col;
        unsigned int line_no = 0;
        unsigned int col_no = 0;

        while (!feof(f)) {
            ++line_no;
            col_no = 0;

            if (fgets(line, _MAX_LINE_LEN, f) == NULL) {
                if (ferror(f))
                    throw ParseException("", line_no, col_no, strerror(ferror(in)));
                else
                    break;
            }

            if (startswith(line, "vt")) {
            } else if (startswith(line, "vp")) {
            } else if (startswith(line, "vn")) {
            } else if (startswith(line, "f")) {
            }
        }
    } catch (ModelException& ex) {
        fclose(f);
        if (model != NULL)
            delete model;
        throw ex;
    }
    return model;
}
