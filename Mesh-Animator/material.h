#include <Eigen/Dense>

#include <fstream>

#include "path.h"

using namespace Eigen;
using namespace std;

struct material {
  string name;
  path filePath, mapKdFilePath;

  // Ka: Ambient RGB color
  // Kd: Diffuse RGB color
  // Ks: Specular RGB color
  Vector3f Ka, Kd, Ks;

  material(const string &mtlFilePath)
      : filePath(mtlFilePath), mapKdFilePath("") {
    ifstream mtlFileStream(mtlFilePath);

    if (!mtlFileStream.good()) {
      throw std::runtime_error("The given .mtl file path is invalid");
    }

    parseMtlFile(mtlFileStream);
  }

  void parseMtlFile(std::ifstream &mtlFileStream) {
    // get material name
    string mtlLabel;
    if (!(mtlFileStream >> mtlLabel >> name)) {
      throw std::runtime_error("mtl parsing error: expected material name");
    }

    // get Ka
    string KaLabel;
    if (!(mtlFileStream >> KaLabel >> Ka[0] >> Ka[1] >> Ka[2])) {
      throw std::runtime_error("mtl parsing error: expected Ka values");
    }

    // get Kd
    string KdLabel;
    if (!(mtlFileStream >> KdLabel >> Kd[0] >> Kd[1] >> Kd[2])) {
      throw std::runtime_error("mtl parsing error: expected Kd values");
    }

    // get Ks
    string KsLabel;
    if (!(mtlFileStream >> KsLabel >> Ks[0] >> Ks[1] >> Ks[2])) {
      throw std::runtime_error("mtl parsing error: expected Ks values");
    }

    // get local texture path
    string mapKdLabel, localMapKdFilePath;
    if (!(mtlFileStream >> mapKdLabel >> localMapKdFilePath)) {
      throw std::runtime_error(
          "mtl parsing error: expected local path to Kd Map");
    }

    //// build full mapKdPath
    // assumption: the given map file path is relative to the mtl file path
    path materialFileParentPath = filePath.getParentPath();

    stringstream ssMaterialFileParentPath(materialFileParentPath.getAsString());

    stringstream ssMapKdFilePath;
    ssMapKdFilePath << ssMaterialFileParentPath.str() << localMapKdFilePath;

    mapKdFilePath = path(ssMapKdFilePath.str());
  }

  path getFilePath() const { return filePath; }

  Vector3f getKa() const { return Ka; }
  Vector3f getKd() const { return Kd; }
  Vector3f getKs() const { return Ks; }

  path getMapKdPath() const { return mapKdFilePath; }
};
