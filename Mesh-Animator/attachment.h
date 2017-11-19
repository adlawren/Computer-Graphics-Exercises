// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cfloat>
#include <climits>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "mesh.h"
#include "skeleton.h"

#ifndef ATTACHMENT_H
#define ATTACHMENT_H

using namespace Eigen;

struct Wtuple {
  unsigned int i, j;
  float w;
};

// attachment class

struct attachment {
  mesh *objp;
  skeleton *skelp;

  MatrixXf *D; // These are the matrices defined in the lecture notes.
  MatrixXi *V;
  MatrixXf *S;
  VectorXf *h;
  MatrixXf *C;
  SparseMatrix<int> *A;
  SparseMatrix<int> *L;
  MatrixXf *W;

  vector<Vector3f> deformedVertexLocations;
  vector<Vector3f> deformedVertexNormals;

  // constructors
  attachment() {}

  attachment(mesh *objref, skeleton *skelref) : objp(objref), skelp(skelref) {}

  // member functions

  void readW(char *);
  void glDrawMeshAttach(bool, int);
  void glDrawDeformedMesh(void);
  void computeDeformedVertices(joint *j, Matrix4f, Vector3f, Quaternionf,
                               int &);

  // computing attachments
  void distancesVisibility(float *);
  void connectionValues(void);
  void adjacencyLaplacian();
  void attachmentWeights(float);
  void enhanceWeights(float, float);
  void connectOrphans();

  // output functions
  void writeVectorXf(VectorXf *, char *);
  void writeMatrixXf(MatrixXf *, char *);
  void writeMatrixXi(MatrixXi *, char *);
  void writeMatrixXfSparse(MatrixXf *, char *);
  void writeSparseMatrixXi(SparseMatrix<int> *, char *);
};

#endif
