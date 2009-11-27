#include <libgen.h>
#include <cstring>
#include <cstdarg>

#include "image.h"

//
// ImageException METHODS
//

ImageException::ImageException(const char*msg_format...) :
    std::exception()
{
  va_list args;
  va_start(args, msg_format);
  vsnprintf(message, 4096, msg_format, args);
  va_end(args);
}


const char* ImageException::what() const throw()
{
  return message;
}


//
// Image METHODS
//

Image::Image(const char *path) throw(ImageException) :
  _type(GL_RGB),
  _texId(0),
  _bytesPerPixel(0),
  _width(0),
  _height(0),
  _pixels(NULL)
{
  const char *filename = basename(const_cast<char *>(path));
  if (filename == NULL)
    throw ImageException("Invalid image filename: %s does not name a file.", filename);

  const char *ext = strrchr(filename, '.');
  if (ext == NULL)
    throw ImageException("Unknown image format.");

  FILE *file = fopen(path, "rb");
  if (file == NULL)
    throw ImageException("File not found: %s.", filename);
  try {
    if (strcasecmp(ext, ".bmp") == 0)
      loadBMP(file);
    else if (strcasecmp(ext, ".tga") == 0)
      loadTGA(file);
    else
      throw ImageException("Unknown image format: %s", ext);
  } catch (ImageException& ex) {
    fclose(file);
    if (_pixels != NULL)
      delete _pixels;
    throw ex;
  }
}


Image::Image(GLenum type, int bytesPerPixel, int width, int height) :
  _type(type),
  _texId(0),
  _bytesPerPixel(bytesPerPixel),
  _width(width),
  _height(height),
  _pixels(NULL)
{
  unsigned int size = _bytesPerPixel * _width * _height;
  _pixels = new unsigned char[size];
}


Image::Image(const Image& img) :
  _type(img._type),
  _texId(0),
  _bytesPerPixel(img._bytesPerPixel),
  _width(img._width),
  _height(img._height),
  _pixels(NULL)
{
    unsigned int size = _bytesPerPixel * _width * _height;
    _pixels = new unsigned char[size];
    memcpy(_pixels, img._pixels, size);
}


Image::~Image()
{
  if (_pixels != NULL)
    delete[] _pixels;
}


void Image::loadBMP(FILE *file) throw(ImageException)
{
    // Read the header data.
    unsigned char file_header[14];
    unsigned char info_header[40];
    if (fread(file_header, sizeof(unsigned char), 14, file) < 14)
        throw ImageException("Invalid or missing texture data.");
    if (fread(info_header, sizeof(unsigned char), 40, file) < 40)
        throw ImageException("Invalid or missing texture data.");

    // TODO: inspect the header data and return suitable errors for unsupported formats.
    _type = GL_BGR;
    _bytesPerPixel = 3;
    _width = (unsigned int)info_header[4] |
             (unsigned int)info_header[5] << 8 |
             (unsigned int)info_header[6] << 16 |
             (unsigned int)info_header[7] << 24;
    _height = (unsigned int)info_header[8] |
              (unsigned int)info_header[9] << 8 |
              (unsigned int)info_header[10] << 16 |
              (unsigned int)info_header[11] << 24;

    // Read the texture data.
    unsigned int numBytes = _width * _height * _bytesPerPixel;
    _pixels = new unsigned char[numBytes];
    if (fread(_pixels, sizeof(unsigned char), numBytes, file) < numBytes)
        throw ImageException("Invalid or missing texture data.");
    // Note that the data from the file is in BGR order.
}


void Image::loadTGA(FILE *file) throw(ImageException)
{
    unsigned char header[18];
    fread(header, sizeof(unsigned char), 18, file);
    if (header[1] != 0) // The colormap byte.
        throw ImageException("Colormap TGA files aren't supported.");

    _width = header[0xC] + header[0xD] * 256; 
    _height = header[0xE] + header[0xF] * 256;
    unsigned int bitDepth = header[0x10];
    /* make sure we are loading a supported bit-depth */
    if (bitDepth != 32 && bitDepth != 24 && bitDepth != 8)
        throw ImageException("TGA files with a bit depth of %d aren't supported.", bitDepth);

    unsigned int numPixels = _width * _height;
    _bytesPerPixel = bitDepth / 8;
    _pixels = new unsigned char[numPixels * _bytesPerPixel];
    switch (header[2]) { // The image type byte
        case 2: // TrueColor, uncompressed
        case 3: // Monochrome, uncompressed
            tgaLoadUncompressed(file, numPixels, _bytesPerPixel, _pixels);
            break;
        case 10: // TrueColor, RLE compressed
        case 11: // Monochrome, RLE compressed
            tgaLoadRLECompressed(file, numPixels, _bytesPerPixel, _pixels);
            break;
        // Unsupported image types.
        default:
            throw ImageException("Unknown TGA image type (type code: %d).", header[2]);
    }

    if (bitDepth == 32)
        _type = GL_BGRA;
    else if (bitDepth == 24)
        _type = GL_BGR;
    else
        _type = GL_ALPHA;
}


void Image::tgaLoadUncompressed(FILE *file, unsigned int numPixels,
        unsigned int bytesPerPixel, unsigned char *pixels)
    throw(ImageException)
{
    unsigned int numBytes = numPixels * bytesPerPixel;
    if (fread(pixels, sizeof(unsigned char), numBytes, file) < numBytes)
        throw ImageException("Missing or invalid TGA image data.");
}


void Image::tgaLoadRLECompressed(FILE *file, unsigned int numPixels,
        unsigned int bytesPerPixel, unsigned char *pixels)
    throw(ImageException)
{
    const int MAX_BYTES_PER_PIXEL = 4;

    int pixelCount;
    bool isEncoded;

    unsigned int pixelsRead = 0;
    unsigned char pixel[MAX_BYTES_PER_PIXEL];
    while (pixelsRead < numPixels) {
        pixelCount = fgetc(file);
        if (pixelCount == EOF)
            throw ImageException("Missing or invalid TGA image data.");

        isEncoded = pixelCount > 127;
        pixelCount = (pixelCount & 0x7F) + 1;
        if (isEncoded) {
            if (fread(pixel, sizeof(unsigned char), bytesPerPixel, file) < bytesPerPixel)
                throw ImageException("Missing or invalid TGA image data.");
            for (int i = 0; i < pixelCount; ++i) {
                memcpy(pixels, pixel, bytesPerPixel);
                pixels += bytesPerPixel;
            }
        } else {
            unsigned int numBytes = pixelCount * bytesPerPixel;
            if (fread(pixels, sizeof(unsigned char), numBytes, file) < numBytes)
                throw ImageException("Missing or invalid TGA image data.");
            pixels += numBytes;
        }
        pixelsRead += pixelCount;
    }
}

