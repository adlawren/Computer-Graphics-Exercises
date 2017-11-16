// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

#include "mesh.h"

using namespace std;
using namespace Eigen;

void mesh::readObjFile(char *fileName) {
  ifstream infile(fileName);
  if (!infile) {
    cerr << "Error: unable to open obj file: " << fileName << endl;
    exit(1);
  }
  string line;

  while (getline(infile, line)) {
    istringstream stin(line);
    string token;

    if (!(stin >> token))
      continue;

    if (token == "mtllib") {
      string localMtlFilePath;
      if (!(stin >> localMtlFilePath))
        throw std::runtime_error("mesh parsing error; expected .mtl file path");

      //// build the full mtl filepath
      // assumption: the given mtl file is relative to the mesh file path
      path objFilePath(fileName);
      path objFileParentPath = objFilePath.getParentPath();

      stringstream ssObjFileParentPath(objFileParentPath.getAsString());

      stringstream ssMtlFilePath;
      ssMtlFilePath << ssObjFileParentPath.str() << localMtlFilePath;

      meshMaterial = new material(ssMtlFilePath.str());
    } else if (token == "v") {
      Vector3f v;
      stin >> v[0] >> v[1] >> v[2];
      vertices.push_back(v);

    } else if (token == "vt") {
      Vector2f vt;
      stin >> vt[0] >> vt[1];
      vertexTextureCoordinates.push_back(vt);
    } else if (token == "vn") {
      Vector3f vn;
      stin >> vn[0] >> vn[1] >> vn[2];
      vertexNormals.push_back(vn);
    } else if (token == "f") {
      vector<Vector3i> face;

      int tmp;
      Vector3i nextVertex;
      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
          stin >> tmp;
          nextVertex[j] = tmp - 1;

          if (j != 2) {
            stin.get(); // skip the '/' characters
          }
        }

        face.push_back(nextVertex);
      }

      faceVertices.push_back(face);
    }
  }
  infile.close();
}

void mesh::writeObjFile(char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }

  outfile << "mtllib " << meshMaterial->getFilePath().getFileName() << endl;

  for (unsigned int i = 0; i < vertices.size(); ++i)
    outfile << "v " << vertices[i][0] << " " << vertices[i][1] << " "
            << vertices[i][2] << endl;
  for (unsigned int i = 0; i < vertexTextureCoordinates.size(); ++i)
    outfile << "vt " << vertexTextureCoordinates[i][0] << " "
            << vertexTextureCoordinates[i][1] << endl;
  for (unsigned int i = 0; i < vertexNormals.size(); ++i)
    outfile << "vn " << vertexNormals[i][0] << " " << vertexNormals[i][1] << " "
            << vertexNormals[i][2] << endl;
  for (unsigned int i = 0; i < faceVertices.size(); ++i) {
    outfile << "f ";
    for (unsigned int j = 0; j < faceVertices[i].size(); ++j) {
      outfile << faceVertices[i][j][0] + 1 << "/" << faceVertices[i][j][1] + 1
              << "/" << faceVertices[i][j][2] + 1;

      // print spaces on all but the last faceVertex value
      if (j != faceVertices[i].size() - 1)
        outfile << " ";
    }
    outfile << endl;
  }
  outfile.close();
}

void mesh::normalize() {
  Vector3f totals(0, 0, 0);
  Vector3f maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  Vector3f mins(FLT_MAX, FLT_MAX, FLT_MAX);

  for (unsigned int v = 0; v < vertices.size(); ++v)
    for (unsigned int j = 0; j < 3; ++j) {
      maxs[j] = max(maxs[j], vertices[v][j]);
      mins[j] = min(mins[j], vertices[v][j]);
      totals[j] += vertices[v][j];
    }
  center = totals / (float)vertices.size();
  Vector3f scales = maxs - mins;
  float scale = (float)scales.maxCoeff();
  meshScale = 1.0 / scale;
}

void mesh::glCreateDisplayList() {
  displayList = glGenLists(1);
  glNewList(displayList, GL_COMPILE);
  for (unsigned int f = 0; f < faceVertices.size(); ++f) {
    glBegin(GL_TRIANGLE_FAN);
    for (unsigned int j = 0; j < faceVertices[f].size(); ++j) {
      unsigned int vertexIndex(faceVertices[f][j][0]);
      unsigned int vertexTextureCoordinateIndex(faceVertices[f][j][1]);
      unsigned int vertexNormalIndex(faceVertices[f][j][2]);

      glNormal3fv(vertexNormals[vertexNormalIndex].data());
      glTexCoord2fv(
          vertexTextureCoordinates[vertexTextureCoordinateIndex].data());
      glVertex3fv(vertices[vertexIndex].data());
    }
    glEnd();
  }
  glEndList();
}

void mesh::glCallDisplayList() {
  if (displayList)
    glCallList(displayList);
  else {
    cerr << "Error: display list incorrectly initialized" << endl;
    exit(1);
  }
}
