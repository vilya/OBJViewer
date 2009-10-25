#ifndef OBJViewer_model_h
#define OBJViewer_model_h

#include <vector>
#include <stdexcept>



const unsigned int _MAX_LINE_LEN = 2048;


class ParseException : public virtual std::exception {
public:
	char message[_MAX_LINE_LEN];

	ParseException(const char *msg_format...);
	virtual ~ParseException() throw() {}
};


struct Vec4 {
    union {
        struct { float x, y, z, w; };
        float data[4];
    };

    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float _x, float _y, float _z, float _w = 1.0) : x(_x), y(_y), z(_z), w(_w) {}
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

    const Vertex& operator [] (unsigned int index) const { return vertexes[index]; }
    Vertex& operator [] (unsigned int index) { return vertexes[index]; }

    unsigned int size() const { return vertexes.size(); }
};


struct Model {
    std::vector<Vec4> v;
    std::vector<Vec4> vt;
    std::vector<Vec4> vp;
    std::vector<Vec4> vn;
    std::vector<Face*> faces;

    Model() : v(), vt(), vp(), vn(), faces() {}
    ~Model() { for (unsigned int i = 0; i < faces.size(); ++i) delete faces[i]; }
};


Model* loadModel(const char* path) throw(ParseException);

#endif // OBJViewer_model_h

