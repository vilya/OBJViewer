#ifndef OBJViewer_image_h
#define OBJViewer_image_h

#include <cstdio>
#include <stdexcept>

#ifdef linux
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif


class ImageException : public std::exception {
public:
	char message[4096];

	ImageException(const char *msg_format...);
	~ImageException() throw() {}

  virtual const char* what() const throw();
};


class Image {
public:
    Image(const char *path) throw(ImageException);
    Image(GLenum type, int bytesPerPixel, int width, int height);
    Image(const Image& img);
    ~Image();

    GLenum getType() const { return _type; }
    unsigned int getBytesPerPixel() const { return _bytesPerPixel; }
    unsigned int getWidth() const { return _width; }
    unsigned int getHeight() const { return _height; }
    unsigned char *getPixels() { return _pixels; }

    unsigned int getTexID() const { return _texId; }
    void setTexID(unsigned int texId) { _texId = texId; }

private:
    void loadBMP(FILE *file) throw(ImageException);
    void loadTGA(FILE *file) throw(ImageException);

    void tgaLoadUncompressed(FILE *file, unsigned int numPixels,
            unsigned int bytesPerPixel, unsigned char *pixels)
        throw(ImageException);

    void tgaLoadRLECompressed(FILE *file, unsigned int numPixels,
            unsigned int bytesPerPixel, unsigned char *pixels)
        throw(ImageException);

private:
    GLenum _type;
    unsigned int _texId;
    unsigned int _bytesPerPixel;
    unsigned int _width;
    unsigned int _height;
    unsigned char *_pixels;
};



#endif // OBJViewer_image_h

