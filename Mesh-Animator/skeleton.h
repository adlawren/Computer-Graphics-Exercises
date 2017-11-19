// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <Eigen/Dense>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "bvh_util.h"
#include "motion.h"
#include "timer.h"

#ifndef SKELETON_H
#define SKELETON_H

#define DEG2RAD_SKE 0.017453292519943 /* rad = deg*PI/180 = deg*DEG2RAD */
#define RAD2DEG_SKE 57.29577951308232 /* deg = rad*180/PI = rad*RAD2DEG */

using namespace std;
using namespace Eigen;

// joint class

enum jointType { rootJoint, regularJoint, endSite };

struct joint {
  string name;
  jointType type;
  unsigned int id;

  Vector3f offset;
  Vector3f rotate, rotateB;
  Vector3f transl, translB;
  Quaternionf quaternion, quaternionB;

  vector<joint *> children;
  vector<string> channelNames;

  // constructor
  joint(string nm, jointType tp) {
    name = nm;
    type = tp;
    offset = Vector3f::Zero();
    rotate = Vector3f::Zero();
    transl = Vector3f::Zero();
    quaternion = Quaternionf::Identity();
    rotateB = Vector3f::Zero();
    translB = Vector3f::Zero();
    quaternionB = Quaternionf::Identity();
  }
};

// skeleton class

struct skeleton {
  joint *root;
  float radius;
  vector<float *> channels, channelsB;
  Vector3i translationIndices;
  float color[4];

  // vertices and bones (used for vertex arrays)
  vector<Vector3f> vertices;
  vector<Vector2i> bones;

  // vertex array
  float *vertexArray;
  unsigned int numVertices;
  unsigned int vaoId[1];

  // constructor
  skeleton() {
    color[1] = color[2] = 0.0;
    color[0] = color[3] = 1.0;
    vertexArray = NULL;
    numVertices = 0;
  }

  // member functions

  void readBvhSkeleton(char *);
  joint *acquire_SubTree(ifstream &, string, jointType);
  void writeBvhSkeleton(char *);
  void output_SubTree(ofstream &, joint *, unsigned int);

  void determineRadius() { radius = maxBranchLength(root); }
  float maxBranchLength(joint *);
  void nullPose();
  void interpolatePose(motion *, double, bool);
  void interpolatePose_SubTree(joint *, float);
  Quaternionf quaternionFromEulers(Vector3f, vector<string>);

  void glColor() { glColor4fv(color); }
  void glDraw() {
    // glDraw_SubTree(root);
    glDraw_SubTree2(root, Matrix4f::Identity());
  }

  void glDraw_SubTree(joint *);
  void glDraw_SubTree2(joint *, Matrix4f);

  // vertices and bones (used for vertex arrays)
  void recoverBones();
  void recoverBones_SubTree(joint *, Vector3f, int);

  void glDrawBones();
  void glDrawBonesHighlight(int);

  // vertex array
  void createVertexArray();
  void glEnableVertexArray();
  void glDrawVertexArray();
  void updateVertexArray();
  void updateVA_SubTree(joint *, Affine3f, Vector3f);

  // VBO
  void glEnableVBO();
  void glUpdateVBO();

  // VAO
  void glEnableVAO();
  void glDrawVAO();
  void glUpdateVAO();
};

#endif
