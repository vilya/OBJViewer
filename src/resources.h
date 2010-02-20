#ifndef OBJViewer_resources_h
#define OBJViewer_resources_h


#include <stdexcept>
#include <string>
#include <vector>


class ResourceException : public virtual std::exception {
public:
  char message[4096];

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
  SearchPath& addAppDir(const std::string& appArgv0, const std::string& relativePath = "");

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

  ResourceManager();
};


// TODO: ResourceContext class, which allows you to push an pop state related to resources?


#endif // OBJViewer_resources_h

