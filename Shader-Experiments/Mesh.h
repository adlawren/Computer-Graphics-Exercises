// Original author: Nathaniel Rossol
// Modified by: Dale Schuurmans

#ifndef MESH_H
#define MESH_H

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Material.h"

using namespace std;

struct Mesh {
  string name;
  string fileName;
  float x, y, z, xscale, yscale, zscale, pitch, yaw, roll;
  Material *material;

  int totalVertices;
  float *vertexPositionsBuffer;
  float *vertexTexCoordsBuffer;
  float *vertexNormalsBuffer;

  Mesh(string file_name, string Name, Material *theMaterial = NULL, float X = 0,
       float Y = 0, float Z = 0, float Xscale = 1.0, float Yscale = 1.0,
       float Zscale = 1.0, float Pitch = 0, float Yaw = 0, float Roll = 0);

  void drawMesh(bool renderOnlyIfOpaque = false,
                bool renderOnlyIfTranslucent = false);

  bool parseMeshFile(string fileName);
};

#endif
