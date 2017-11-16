#pragma once

#include <sstream>

using namespace std;

struct path {
  stringstream ssPath;

  path(const string &pathStr) {
    ssPath.str(pathStr);
    ssPath.clear();
  }

  // CC
  path(const path &otherPath) {
    ssPath.str(otherPath.ssPath.str());
    ssPath.clear();
  }

  // AO
  path &operator=(const path &otherPath) {
    ssPath.str(otherPath.ssPath.str());
    ssPath.clear();
  }

  string getAsString() const { return ssPath.str(); }

  path getParentPath() const {
    stringstream ssPathCopy(ssPath.str()), ssParentPath;

    // build a stringstream one character at a time until the next character
    // is a forward slash, or there no more characters.
    // -> if the next character is a slash, append the stringstream to the
    // parent path stringstream (else, do nothing; prune the last part of the
    // file path).
    stringstream ssNextFolderName;
    char nextChar;
    for (char nextChar : ssPathCopy.str()) {
      if (nextChar != '/') {
        ssNextFolderName << nextChar;
      } else {
        ssParentPath << ssNextFolderName.str() << "/";

        // reset stringstream
        ssNextFolderName.str("");
        ssNextFolderName.clear();
      }
    }

    return path(ssParentPath.str());
  }

  string getFileName() const {
    stringstream ssPathCopy(ssPath.str()), ssParentPath;

    stringstream ssNextFolderName;
    char nextChar;
    for (char nextChar : ssPathCopy.str()) {
      if (nextChar != '/') {
        ssNextFolderName << nextChar;
      } else {
        ssParentPath << ssNextFolderName.str() << "/";

        // reset stringstream
        ssNextFolderName.str("");
        ssNextFolderName.clear();
      }
    }

    return ssNextFolderName.str();
  }
};
