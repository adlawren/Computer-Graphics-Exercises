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

    if (token == "v") {
      Vector3f v;
      stin >> v[0] >> v[1] >> v[2];
      vertices.push_back(v);

    } else if (token == "f") {
      vector<unsigned int> faceV;
      unsigned int i;
      while (stin >> i) {
        faceV.push_back(i - 1);
        // temporarily ignoring texture/normal vertices; parsing over them
        if (stin.get() == '/') {
          if (stin.peek() == '/') {
            stin.get();
            stin >> i;
            continue;
          }
          stin >> i;
          if (stin.get() == '/')
            stin >> i;
        }
      }
      faceVertices.push_back(faceV);
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
  for (unsigned int i = 0; i < vertices.size(); ++i)
    outfile << "v " << vertices[i][0] << " " << vertices[i][1] << " "
            << vertices[i][2] << endl;
  for (unsigned int i = 0; i < faceVertices.size(); ++i) {
    outfile << "f ";
    for (unsigned int j = 0; j < faceVertices[i].size(); ++j) {
      outfile << faceVertices[i][j] + 1;
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
      unsigned int v(faceVertices[f][j]);
      glVertex3fv(vertices[v].data());
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

void mesh::createVertexArray() {
  numVertices = vertices.size();
  vertexArray = new float[3 * numVertices];

  for (unsigned int v = 0; v < numVertices; ++v) {
    vertexArray[3 * v + 0] = vertices[v][0];
    vertexArray[3 * v + 1] = vertices[v][1];
    vertexArray[3 * v + 2] = vertices[v][2];
  }

  numTriangles = 0;
  for (unsigned int f = 0; f < faceVertices.size(); ++f)
    numTriangles += faceVertices[f].size() - 2;

  triangleIndices = new unsigned int[3 * numTriangles];
  diameters = new float[numTriangles];
  int t(0);

  for (unsigned int f = 0; f < faceVertices.size(); ++f) {
    // trianglulate each face
    vector<unsigned int> face(faceVertices[f]);
    unsigned int i0(face[0]);
    unsigned int i1;
    unsigned int i2;

    for (unsigned int i = 1; i < face.size() - 1; ++i) {
      // triangle fan
      i1 = face[i];
      i2 = face[i + 1];
      triangleIndices[3 * t + 0] = i0;
      triangleIndices[3 * t + 1] = i1;
      triangleIndices[3 * t + 2] = i2;

      float diameter(0);
      Vector3f p0(vertices[i0]);
      Vector3f p1(vertices[i1]);
      Vector3f p2(vertices[i2]);
      diameter = max(diameter, sqrt((p1 - p0).dot(p1 - p0)));
      diameter = max(diameter, sqrt((p2 - p1).dot(p2 - p1)));
      diameter = max(diameter, sqrt((p0 - p2).dot(p0 - p2)));
      diameters[t] = diameter;

      ++t;
    }
  }
}

void mesh::glEnableVertexArray() {
  if (vertexArray == NULL) {
    cerr << "Error: vertex array incorrectly initialized" << endl;
    exit(1);
  }
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, vertexArray);
}

void mesh::glDrawVertexArray() {
  if (triangleIndices == NULL) {
    cerr << "Error: triangle indices incorrectly initialized" << endl;
    exit(1);
  } else
    glDrawElements(GL_TRIANGLES, 3 * numTriangles, GL_UNSIGNED_INT,
                   triangleIndices);
}

void mesh::glEnableVBO() {
  unsigned int bufferIds[2];
  glGenBuffers(2, bufferIds);

  // vertices
  glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * numVertices * sizeof(float), vertexArray,
               GL_STATIC_DRAW);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  // triangleIndices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * numTriangles * sizeof(unsigned int),
               triangleIndices, GL_STATIC_DRAW);
}

void mesh::glDrawVBO() {
  glDrawElements(GL_TRIANGLES, 3 * numTriangles, GL_UNSIGNED_INT, 0);
}

void mesh::glEnableVAO() {

#ifdef __APPLE__
  glGenVertexArraysAPPLE(1, vaoId);
  glBindVertexArrayAPPLE(vaoId[0]);
#else
  glGenVertexArrays(1, vaoId);
  glBindVertexArray(vaoId[0]);
#endif
  glEnableVBO();
}

void mesh::glDrawVAO() {
#ifdef __APPLE__
  glBindVertexArrayAPPLE(vaoId[0]);
#else
  glBindVertexArray(vaoId[0]);
#endif
  glDrawVBO();
}
