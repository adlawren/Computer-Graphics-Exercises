// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

// attachGenerator <mesh.obj> <motion.bvh>

#include <iostream>
#include <Eigen/Dense>

#include "mesh.h"
#include "skeleton.h"
#include "attachment.h"

using namespace Eigen;

/* global variables */
mesh obj;
skeleton skel;
attachment att(&obj, &skel);
float boneRadii[] = {2.0,  2.0, 1.0, 0.5, 0.25, 0.0,  2.0, 2.0, 1.0, 0.5,
                     0.25, 0.0, 2.0, 2.0, 2.5,  1.5,  0.5, 0.5, 0.5, 2.0,
                     1.0,  1.0, 0.5, 0.5, 0.5,  0.25, 0.0, 0.0, 2.0, 1.0,
                     1.0,  0.5, 0.5, 0.5, 0.25, 0.0,  0.0};

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "Usage: attachGenerator <meshfile.obj> <motionfile.bvh>" << endl;
    exit(1);
  }

  // mesh
  obj.readObjFile(argv[1]);
  obj.normalize();
  obj.createVertexArray();

  // skeleton
  skel.readBvhSkeleton(argv[2]);
  skel.determineRadius();
  skel.recoverBones();

  // attachment
  att.distancesVisibility(boneRadii);
  att.connectionValues();
  att.adjacencyLaplacian();
  att.attachmentWeights(1.0);
  att.enhanceWeights(5.0, 0.1);
  att.connectOrphans();

  // output
  att.writeMatrixXfSparse(att.W, "W.out");

  return 0;
}
