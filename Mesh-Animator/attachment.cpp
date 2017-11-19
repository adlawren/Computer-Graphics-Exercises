// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

#include "attachment.h"

using namespace Eigen;

const float TOL(0.0000001);

void attachment::readW(char *fileName) {
  ifstream infile(fileName);
  if (!infile) {
    cerr << "Error: unable to open input file: " << fileName << endl;
    exit(1);
  }

  string line;
  vector<Wtuple> data;
  unsigned int i;

  // gather data tuples (i,j,val)
  // assume line format: i (j,val) ... (j,val)
  // assume i's complete and ordered
  while (getline(infile, line)) {
    istringstream stin(line);
    char token;
    stin >> i;

    while (stin >> token) {
      Wtuple entry;
      entry.i = i;
      if (token != '(') {
        cerr << "Error: '(' expected but '" << token << "' encountered" << endl;
        exit(1);
      }
      stin >> entry.j;
      stin >> token;
      if (token != ',') {
        cerr << "Error: ',' expected but '" << token << "' encountered" << endl;
        exit(1);
      }
      stin >> entry.w;
      stin >> token;
      if (token != ')') {
        cerr << "Error: ')' expected but '" << token << "' encountered" << endl;
        exit(1);
      }
      data.push_back(entry);
    }
  }

  W = new MatrixXf(objp->vertices.size(), skelp->bones.size());
  (*W).setZero(W->rows(), W->cols());

  for (unsigned int t = 0; t < data.size(); ++t)
    (*W)(data[t].i - 1, data[t].j - 1) = data[t].w;
}

void attachment::glDrawMeshAttach(bool highlightMode, int highlightBone) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (unsigned int f = 0; f < objp->faceVertices.size(); ++f) {
    glBegin(GL_TRIANGLE_FAN);
    for (unsigned int j = 0; j < objp->faceVertices[f].size(); ++j) {
      unsigned int vertexIndex(objp->faceVertices[f][j][0]);
      unsigned int vertexTextureCoordinateIndex(objp->faceVertices[f][j][1]);
      unsigned int vertexNormalIndex(objp->faceVertices[f][j][2]);

      float val = 0.0f;
      if (highlightMode)
        val = (float)(*V)(vertexIndex, highlightBone);
      else
        val = (*W)(vertexIndex, highlightBone);
      glColor3f(0.9 * val + 0.1, 0.9 * val + 0.1, 0.9 * val + 0.1);

      glNormal3fv(objp->vertexNormals[vertexNormalIndex].data());
      glTexCoord2fv(
          objp->vertexTextureCoordinates[vertexTextureCoordinateIndex].data());
      glVertex3fv(objp->vertices[vertexIndex].data());
    }
    glEnd();
  }
}

void attachment::computeDeformedVertices(joint *current,
                                         Matrix4f transformationMatrix,
                                         Vector3f accumulatedOffset,
                                         Quaternionf accumulatedRotation,
                                         int &nodeCount) {
  glPushMatrix();

  if (current->type == rootJoint) {
    Matrix4f rootTranslationMatrix = Matrix4f::Identity();
    rootTranslationMatrix(0, 3) = current->transl(0);
    rootTranslationMatrix(1, 3) = current->transl(1);
    rootTranslationMatrix(2, 3) = current->transl(2);

    transformationMatrix *= rootTranslationMatrix;
  } else {
    //// transform the mesh vertices based on accumulated transformations and
    //// bone weights
    for (unsigned int vertexIndex = 0; vertexIndex < objp->vertices.size();
         ++vertexIndex) {
      float weight = (*W)(vertexIndex, nodeCount - 1);
      if (weight > 0.0f) {
        //// deform vertex location
        Vector4f vertexLocation = Vector4f::Zero();
        vertexLocation.segment(0, 3) =
            objp->vertices[vertexIndex] - accumulatedOffset;
        vertexLocation[3] = 1.0f;
        deformedVertexLocations[vertexIndex] +=
            (transformationMatrix * vertexLocation).segment(0, 3) * weight;

        // deform vertex normal
        deformedVertexNormals[vertexIndex] +=
            accumulatedRotation.toRotationMatrix() *
            objp->vertexNormals[vertexIndex] * weight;
      }
    }
  }

  Matrix4f offsetTranslationMatrix = Matrix4f::Identity();
  offsetTranslationMatrix(0, 3) = current->offset(0);
  offsetTranslationMatrix(1, 3) = current->offset(1);
  offsetTranslationMatrix(2, 3) = current->offset(2);

  transformationMatrix *= offsetTranslationMatrix;

  accumulatedOffset += current->offset;

  Matrix4f intermediateRotationMatrix = Matrix4f::Identity();
  intermediateRotationMatrix.block(0, 0, 3, 3) =
      current->quaternion.toRotationMatrix();

  transformationMatrix *= intermediateRotationMatrix;
  accumulatedRotation *= current->quaternion;

  nodeCount += 1;

  for (unsigned int c = 0; c < current->children.size(); ++c)
    computeDeformedVertices(current->children[c], transformationMatrix,
                            accumulatedOffset, accumulatedRotation, nodeCount);

  glPopMatrix();
}

void attachment::glDrawDeformedMesh() {
  // initialize deformed vertices
  deformedVertexLocations = objp->vertices;
  deformedVertexNormals = objp->vertexNormals;

  // move the vertices to the origin and scale
  for (int vertexIndex = 0; vertexIndex < deformedVertexLocations.size();
       ++vertexIndex) {
    deformedVertexLocations[vertexIndex] -= objp->center;
    deformedVertexLocations[vertexIndex] *= objp->meshScale;
  }

  // deform vertices; recurse over skeleton
  int nodeCount = 0;
  computeDeformedVertices(skelp->root, Matrix4f::Identity(), Vector3f::Zero(),
                          Quaternionf::Identity(), nodeCount);

  // draw deformed vertices
  for (unsigned int f = 0; f < objp->faceVertices.size(); ++f) {
    glBegin(GL_TRIANGLE_FAN);
    for (unsigned int j = 0; j < objp->faceVertices[f].size(); ++j) {
      unsigned int vertexIndex(objp->faceVertices[f][j][0]);
      unsigned int vertexTextureCoordinateIndex(objp->faceVertices[f][j][1]);
      unsigned int vertexNormalIndex(objp->faceVertices[f][j][2]);

      glColor3f(1.0, 1.0, 1.0);
      glNormal3fv(deformedVertexNormals[vertexNormalIndex].data());
      glTexCoord2fv(
          objp->vertexTextureCoordinates[vertexTextureCoordinateIndex].data());
      glVertex3fv(deformedVertexLocations[vertexIndex].data());
    }
    glEnd();
  }
}

/*********************************************/
/* Extra functions for computing attachments */
/*********************************************/

void attachment::distancesVisibility(float *boneRadii) {
  D = new MatrixXf(objp->vertices.size(), skelp->bones.size());
  V = new MatrixXi(objp->vertices.size(), skelp->bones.size());

  // loop through bones
  for (unsigned int j = 0; j < skelp->bones.size(); ++j) {

    cout << "computing distances/visibilities to bone " << j + 1 << endl;

    unsigned int v0 = skelp->bones[j][0];
    unsigned int v1 = skelp->bones[j][1];
    Vector3f orig = skelp->vertices[v0];
    Vector3f bone = skelp->vertices[v1] - orig;
    float bone_dot_bone = bone.dot(bone);
    Vector3f bone_normed = bone / bone_dot_bone;

    // loop through vertices
    for (unsigned int i = 0; i < objp->vertices.size(); ++i) {

      Vector3f point = objp->vertices[i] - orig;
      float point_dot_bone = point.dot(bone);

      float delta[3], p0[3]; // using raw arrays for efficiency

      if (point_dot_bone <= 0) {
        p0[0] = orig[0];
        p0[1] = orig[1];
        p0[2] = orig[2];
        delta[0] = point[0];
        delta[1] = point[1];
        delta[2] = point[2];
      } else if (point_dot_bone >= bone_dot_bone) {
        p0[0] = bone[0] + orig[0];
        p0[1] = bone[1] + orig[1];
        p0[2] = bone[2] + orig[2];
        delta[0] = point[0] - bone[0];
        delta[1] = point[1] - bone[1];
        delta[2] = point[2] - bone[2];
      } else {
        float proj[3];
        proj[0] = point_dot_bone * bone_normed[0];
        proj[1] = point_dot_bone * bone_normed[1];
        proj[2] = point_dot_bone * bone_normed[2];
        p0[0] = proj[0] + orig[0];
        p0[1] = proj[1] + orig[1];
        p0[2] = proj[2] + orig[2];
        delta[0] = point[0] - proj[0];
        delta[1] = point[1] - proj[1];
        delta[2] = point[2] - proj[2];
      }

      float d_sq =
          delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2];
      float dist = sqrt(d_sq);
      (*D)(i, j) = d_sq;

      if (d_sq > boneRadii[j])
        continue; // too far to be considered visible

      // prepare for visibility checking

      float p1[3], p2[3], p3[3], dvec0[3], dvec1[3];
      float a00, a01, a02, a10, a11, a12, a20, a21, a22;
      float d0, d1, d2, d3, beta, gamma, lambda;
      float dot1, dot2, dot3;
      unsigned int v1, v2, v3;
      bool visible(true);

      // loop through triangles
      for (unsigned int f = 0; f < objp->numTriangles; ++f) {

        v1 = objp->triangleIndices[3 * f + 0];
        p1[0] = objp->vertexArray[3 * v1 + 0] - p0[0];
        p1[1] = objp->vertexArray[3 * v1 + 1] - p0[1];
        p1[2] = objp->vertexArray[3 * v1 + 2] - p0[2];

        if (sqrt(p1[0] * p1[0] + p1[1] * p1[1] + p1[2] * p1[2]) >
            dist + objp->diameters[f])
          continue; // triangle too far away

        v2 = objp->triangleIndices[3 * f + 1];
        v3 = objp->triangleIndices[3 * f + 2];
        p2[0] = objp->vertexArray[3 * v2 + 0] - p0[0];
        p2[1] = objp->vertexArray[3 * v2 + 1] - p0[1];
        p2[2] = objp->vertexArray[3 * v2 + 2] - p0[2];
        p3[0] = objp->vertexArray[3 * v3 + 0] - p0[0];
        p3[1] = objp->vertexArray[3 * v3 + 1] - p0[1];
        p3[2] = objp->vertexArray[3 * v3 + 2] - p0[2];

        dot1 = p1[0] * delta[0] + p1[1] * delta[1] + p1[2] * delta[2];
        dot2 = p2[0] * delta[0] + p2[1] * delta[1] + p2[2] * delta[2];
        dot3 = p3[0] * delta[0] + p3[1] * delta[1] + p3[2] * delta[2];

        if ((dot1 <= 0 && dot2 <= 0 && dot3 <= 0) ||
            (dot1 >= d_sq && dot2 >= d_sq && dot3 >= d_sq))
          continue;
        // triangle entirely behind or beyond delta segment

        a00 = p1[0] - p2[0];
        a10 = p1[1] - p2[1];
        a20 = p1[2] - p2[2];
        a01 = p1[0] - p3[0];
        a11 = p1[1] - p3[1];
        a21 = p1[2] - p3[2];
        a02 = p1[0];
        a12 = p1[1];
        a22 = p1[2];

        dvec0[0] = a11 * delta[2] - delta[1] * a21;
        dvec0[1] = delta[0] * a21 - a01 * delta[2];
        dvec0[2] = a01 * delta[1] - a11 * delta[0];
        d0 = a00 * dvec0[0] + a10 * dvec0[1] + a20 * dvec0[2];
        d1 = a02 * dvec0[0] + a12 * dvec0[1] + a22 * dvec0[2];

        if ((d1 >= -TOL && (d0 <= TOL || d1 >= d0 - TOL)) ||
            (d1 <= TOL && (d0 >= -TOL || d1 <= d0 + TOL)))
          continue; // beta failed

        dvec1[0] = a10 * a22 - a12 * a20;
        dvec1[1] = a02 * a20 - a00 * a22;
        dvec1[2] = a00 * a12 - a02 * a10;
        d2 = delta[0] * dvec1[0] + delta[1] * dvec1[1] + delta[2] * dvec1[2];

        if ((d2 >= -TOL && (d0 <= TOL || d2 >= d0 - TOL)) ||
            (d2 <= TOL && (d0 >= -TOL || d2 <= d0 + TOL)) || // gamma failed
            ((d1 + d2) / d0 >= 1 - TOL))
          continue; // beta+gamma failed

        d3 = -a01 * dvec1[0] - a11 * dvec1[1] - a21 * dvec1[2];

        if ((d3 >= -TOL && (d0 <= TOL || d3 >= d0 - TOL)) ||
            (d3 <= TOL && (d0 >= -TOL || d3 <= d0 + TOL)))
          continue; // lambda failed

        // otherwise, found an occluding face
        visible = false;
        break;
      }

      if (visible)
        (*V)(i, j) = 1;
    }
  }
  cout << "done computing distances/visibilities" << endl;
}

void attachment::connectionValues() {
  S = new MatrixXf(D->rows(), D->cols());
  h = new VectorXf(D->rows());
  C = new MatrixXf(D->rows(), D->cols());
  float val;

  for (unsigned int i = 0; i < D->rows(); ++i) {

    // compute S(i,:)
    float recip(0);
    for (unsigned int j = 0; j < D->cols(); ++j) {
      val = !(*V)(i, j) ? 100000000.0 : (*D)(i, j);
      (*S)(i, j) = val;
      recip += 1.0 / val;
    }
    // compute h(i)
    (*h)(i) = recip;

    // compute C(i,:)
    float val, rowtotal(0);
    for (unsigned int j = 0; j < D->cols(); ++j) {
      val = 1.0 / ((*h)(i) * (*S)(i, j));
      rowtotal += val;
      (*C)(i, j) = val;
    }
    // renormalize C(i,:), just to make sure
    for (unsigned int j = 0; j < D->cols(); ++j)
      (*C)(i, j) /= rowtotal;
  }
}

void attachment::adjacencyLaplacian() {
  A = new SparseMatrix<int>(objp->vertices.size(), objp->vertices.size());
  A->reserve(2 * 3 * objp->faceVertices.size());

  // sparse adjacency matrix A
  for (unsigned int f = 0; f < objp->numTriangles; ++f) {
    unsigned int v0(objp->triangleIndices[3 * f + 0]);
    unsigned int v1(objp->triangleIndices[3 * f + 1]);
    unsigned int v2(objp->triangleIndices[3 * f + 2]);

    if (!A->coeffRef(v0, v1) && v0 != v1) {
      A->coeffRef(v0, v1) = 1;
      A->coeffRef(v1, v0) = 1;
    }
    if (!A->coeffRef(v1, v2) && v1 != v2) {
      A->coeffRef(v1, v2) = 1;
      A->coeffRef(v2, v1) = 1;
    }
    if (!A->coeffRef(v2, v0) && v2 != v0) {
      A->coeffRef(v2, v0) = 1;
      A->coeffRef(v0, v2) = 1;
    }
  }
  // sparse Laplacian matrix L
  L = new SparseMatrix<int>(objp->vertices.size(), objp->vertices.size());
  L->reserve(2 * objp->numTriangles + objp->vertices.size());
  *L = -*A;
  for (unsigned int i = 0; i < A->outerSize(); ++i) {
    int rowsum(0);
    for (SparseMatrix<int>::InnerIterator t(*A, i); t; ++t)
      rowsum += t.value();
    L->coeffRef(i, i) = rowsum;
  }
}

void attachment::attachmentWeights(float eta) {
  // build linear system matrix X = eta*L + Delta(h)
  SparseMatrix<float, ColMajor> X(L->rows(), L->cols());
  X = eta * L->cast<float>();
  for (unsigned int i = 0; i < L->cols(); ++i)
    X.coeffRef(i, i) += (*h)(i);
  X.makeCompressed();

  // define solver
  SimplicialLDLT<SparseMatrix<float>> solver;
  solver.compute(X);
  if (solver.info() != Success) {
    cerr << "Error: solver failed to decompose X" << endl;
    exit(1);
  }
  // define target matrix Delta(h)C and weight matrix W
  MatrixXf hC = h->asDiagonal() * (*C);
  W = new MatrixXf(C->rows(), C->cols());

  // solve for W, one column at a time
  for (unsigned int j = 0; j < C->cols(); ++j) {
    W->col(j) = solver.solve(hC.col(j));
    if (solver.info() != Success) {
      cerr << "Error: solver failed on" << j << "th column" << endl;
      exit(1);
    }
  }
  // renormalize rows of W, just to make sure
  for (unsigned int i = 0; i < W->rows(); ++i) {
    float rowtotal(0.0);
    for (unsigned int j = 0; j < W->cols(); ++j)
      rowtotal += (*W)(i, j);
    for (unsigned int j = 0; j < W->cols(); ++j)
      (*W)(i, j) /= rowtotal;
  }
}

void attachment::enhanceWeights(float power, float threshold) {
  float rowtotal;

  for (unsigned int i = 0; i < W->rows(); ++i) {
    // exponentiate and renormalize
    rowtotal = 0;
    for (unsigned int j = 0; j < W->cols(); ++j) {
      (*W)(i, j) = pow((*W)(i, j), power);
      rowtotal += (*W)(i, j);
    }
    for (unsigned int j = 0; j < W->cols(); ++j)
      (*W)(i, j) /= rowtotal;
    // threshold and renormalize
    rowtotal = 0;
    for (unsigned int j = 0; j < W->cols(); ++j) {
      (*W)(i, j) = ((*W)(i, j) >= threshold) ? (*W)(i, j) : 0;
      rowtotal += (*W)(i, j);
    }
    for (unsigned int j = 0; j < W->cols(); ++j)
      (*W)(i, j) /= rowtotal;
  }
}

void attachment::connectOrphans() {
  vector<float> rowtotals;
  for (unsigned int i = 0; i < W->rows(); ++i) {
    float rowtotal(0.0);
    for (unsigned int j = 0; j < W->cols(); ++j)
      rowtotal += (*W)(i, j);
    rowtotals.push_back(rowtotal);
  }

  for (unsigned int i = 0; i < W->rows(); ++i)
    if (isnan(rowtotals[i])) { // orphan
      Vector3f vertexi(objp->vertices[i]);
      float mindist(FLT_MAX);
      unsigned int vmin(-1);

      // find nearest vertex
      for (unsigned int v = 0; v < objp->vertices.size(); ++v)
        if (!isnan(rowtotals[v]) && v != i) {
          Vector3f delta(vertexi - objp->vertices[v]);
          float dist(delta.norm());
          if (dist < mindist) {
            mindist = dist;
            vmin = v;
          }
        };
      // copy attachments
      for (unsigned int j = 0; j < W->cols(); ++j)
        (*W)(i, j) = (*W)(vmin, j);
    };
}

/******************************/
/* Extra functions for output */
/******************************/

void attachment::writeVectorXf(VectorXf *v, char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }
  for (unsigned int i = 0; i < v->size(); ++i)
    outfile << i + 1 << " " << (*v)(i) << endl;
}

void attachment::writeMatrixXf(MatrixXf *M, char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }
  for (unsigned int i = 0; i < M->rows(); ++i) {
    outfile << i + 1 << " ";
    for (unsigned int j = 0; j < M->cols(); ++j)
      outfile << (*M)(i, j) << " ";
    outfile << endl;
  }
}

void attachment::writeMatrixXi(MatrixXi *M, char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }
  for (unsigned int i = 0; i < M->rows(); ++i) {
    outfile << i + 1 << " ";
    for (unsigned int j = 0; j < M->cols(); ++j)
      outfile << (*M)(i, j) << " ";
    outfile << endl;
  }
}

void attachment::writeMatrixXfSparse(MatrixXf *M, char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }
  for (unsigned int i = 0; i < M->rows(); ++i) {
    outfile << i + 1 << " ";
    for (unsigned int j = 0; j < M->cols(); ++j) {
      if ((*M)(i, j) > 0)
        outfile << "(" << j + 1 << "," << (*M)(i, j) << ") ";
    }
    outfile << endl;
  }
}

void attachment::writeSparseMatrixXi(SparseMatrix<int> *M, char *fileName) {
  ofstream outfile(fileName);
  if (!outfile) {
    cerr << "Error: unable to open output file: " << fileName << endl;
    exit(1);
  }
  for (unsigned int i = 0; i < M->outerSize(); ++i) {
    outfile << i + 1 << " ";
    for (SparseMatrix<int>::InnerIterator t(*M, i); t; ++t) {
      outfile << "(" << t.row() + 1 << "," << t.value() << ") ";
    }
    outfile << endl;
  }
}
