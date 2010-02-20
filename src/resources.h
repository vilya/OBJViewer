#ifndef OBJViewer_resources_h
#define OBJViewer_resources_h


#include <stdexcept>
#include <string>
#include <vector>


const unsigned int _MAX_LINE_LEN = 4096;


class ResourceException : public virtual std::exception {
public:
  char message[_MAX_LINE_LEN];

  ResourceException(const char *msg_format...);
  virtual ~ResourceException() throw() {}

  virtual const char* what() const throw();
};


class SearchPath {
public:
  SearchPath();
  SearchPath(const std::string& path);
  SearchPath(const SearchPath& other);

  SearchPath& addDir(const std::string& dir);
  SearchPath& addCurrentDir();

  std::string find(const std::string& relativePath) const throw(ResourceException);

private:
  std::string normalisePath(const std::string& path) const;
  bool isRelativePath(const std::string& path) const;

private:
  std::vector<std::string> _dirs;
};


class ResourceManager {
public:
  SearchPath textures;
  SearchPath shaders;

  ResourceManager(const std::string appPath);
};


// TODO: ResourceContext class, which allows you to push an pop state related to resources?


#endif // OBJViewer_resources_h

