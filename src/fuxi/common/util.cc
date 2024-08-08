
#include "fuxi/common/util.h"

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <string>

namespace next {
namespace fuxi {

/*static*/ bool Util::EnsureDirectory(const std::string& directory_path) {
  if (DirectoryExists(directory_path)) {
    return true;
  }
  std::string path = directory_path;
  for (size_t i = 1; i < directory_path.size(); ++i) {
    if (directory_path[i] == '/') {
      // Whenever a '/' is encountered, create a temporary view from
      // the start of the path to the character right before this.
      path[i] = 0;

      if (mkdir(path.c_str(), S_IRWXU) != 0) {
        if (errno != EEXIST) {
          return false;
        }
      }

      // Revert the temporary view back to the original.
      path[i] = '/';
    }
  }

  // Make the final (full) directory.
  if (mkdir(path.c_str(), S_IRWXU) != 0) {
    if (errno != EEXIST) {
      return false;
    }
  }

  return true;
}

/*static*/ bool Util::DirectoryExists(const std::string& directory_path) {
  struct stat info;
  if (stat(directory_path.c_str(), &info) != 0) {
    return false;
  }

  if (info.st_mode & S_IFDIR) {
    return true;
  }

  return false;
}

}  // namespace fuxi
}  // namespace next
