#include <cstdio>
#include <unistd.h> // for getcwd()
#include <libgen.h> // for dirname()

#include "resources.h"



//
// ResourceException methods
//

ResourceException::ResourceException(const char *msg_format...) :
  std::exception()
{
  va_list args;
  va_start(args, msg_format);
  vsnprintf(message, sizeof(message), msg_format, args);
  va_end(args);
}


const char* ResourceException::what() const throw()
{
  return message;
}


//
// SearchPath methods
//

SearchPath::SearchPath() : _dirs()
{
}


SearchPath::SearchPath(const std::string& path) : _dirs()
{
  size_t pos = 0;
  while (pos < path.size()) {
    size_t end = path.find(':', pos);
    if (end == std::string::npos)
      end = path.size();
    if (end > pos)
      addDir(path.substr(pos, end - pos));
    pos = end + 1;
  }
}


SearchPath::SearchPath(const SearchPath& other) : _dirs(other._dirs)
{
}


SearchPath& SearchPath::addDir(const std::string& dir)
{
  std::string normalisedDir = normalisePath(dir);
  if (std::find(_dirs.begin(), _dirs.end(), normalisedDir) == _dirs.end())
    _dirs.push_back(normalisedDir);
  return *this;
}


SearchPath& SearchPath::addCurrentDir()
{
  char currentDirBuf[2048];
  if (getcwd(currentDirBuf, sizeof(currentDirBuf)) != NULL) {
    std::string currentDir(currentDirBuf);
    if (std::find(_dirs.begin(), _dirs.end(), currentDir) == _dirs.end())
      _dirs.push_back(currentDir);
  }
  return *this;
}


SearchPath& SearchPath::addAppDir(const std::string& appArgv0, const std::string& relativePath)
{
  std::string appDir = dirname(const_cast<char*>(appArgv0.c_str()));
  appDir = appDir + "/" + relativePath;
  return addDir(appDir);
}


std::string SearchPath::find(const std::string& filename) const throw(ResourceException)
{
  std::string normalisedFilename = normalisePath(filename);
  if (!isRelativePath(normalisedFilename))
    return normalisedFilename;

  for (size_t i = 0; i < _dirs.size(); ++i) {
    std::string fullPath = _dirs[i] + "/" + normalisedFilename;
    FILE* f = fopen(fullPath.c_str(), "rb");
    if (f != NULL) {
      fclose(f);
      return fullPath;
    }
  }

  throw ResourceException("%s not found", filename.c_str());
}


std::string SearchPath::normalisePath(const std::string& path) const
{
  char buf[2048];
  size_t bufIndex = 0;

  for (size_t pathIndex = 0; pathIndex < path.size(); ++pathIndex) {
    char ch = path[pathIndex];
    if (ch == '/') {
      if (bufIndex == 0 || buf[bufIndex-1] != '/')
        buf[bufIndex++] = ch;
    }
  }

  if (bufIndex > 0 && buf[bufIndex-1] == '/')
    --bufIndex;

  buf[bufIndex] = '\0';
  return std::string(buf);
}


bool SearchPath::isRelativePath(const std::string& path) const
{
  return (path.size() == 0 || path[0] != '/');
}


//
// ResourceManager methods
//

ResourceManager::ResourceManager() : textures(), shaders()
{
}

