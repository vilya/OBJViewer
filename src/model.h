#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <list>
#include <vector>
#include <stdexcept>



const unsigned int _MAX_LINE_LEN = 2048;


class AnyException : public virtual std::exception {
public:
	char message[_MAX_LINE_LEN];

	AnyException(const char *msg_format...);
	virtual ~AnyException() throw() {}
};


class ParseException : public virtual std::exception {
public:
	char line[_MAX_LINE_LEN];
	char message[_MAX_LINE_LEN];
	unsigned int line_number, col_number;

	ParseException(const char *a_line, unsigned int a_line_number, unsigned int a_col_number, const char *msg_format...);
	virtual ~ParseException() throw() {}
};


struct Vec4 {
    union {
        struct { float x, y, z, w; }
        float data[4];
    };

    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vec4(const Vec4& vec): x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}

    float operator [] (int index) const { return data[index]; }
    float& operator [] (int index) { return data[index]; }
};



struct Vertex {
    int v, vt, vn;

    Vertex(int _v, int _vt, int _vn) : v(_v), vt(_vt), vn(_vn) {}
};


struct Face {
    std::vector<Vertex> vertexes;
};


struct Model {
    std::vector<Vec4> v;
    std::vector<Vec4> vt;
    std::vector<Vec4> vp;
    std::vector<Vec4> vn;
    std::list<Face> faces;

    Model() : v(), vt(), vp(), vn() {}
    ~Model() {}
};


Model* loadModel(const char* path) throw(ModelException);

#endif // OBJViewer_model_h

