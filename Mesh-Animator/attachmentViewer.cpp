// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

// attachmentViewer <mesh.obj> <motion.bvh> <weights.att>

#include <Eigen/Dense>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "camera.h"
#include "mesh.h"
#include "skeleton.h"
#include "attachment.h"

using namespace Eigen;

int main(int, char **);
void setup(char *, char *, char *);
void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int, int, int);

/* global variables */
camera cam;
mesh obj;
skeleton skel;
attachment att(&obj, &skel);
int highlightBone(0);
bool highlightMode = false;
float boneRadii[] = {0.5,  2.0, 1.0, 0.5, 0.25, 0.0,  0.5, 2.0, 1.0, 0.5,
                     0.25, 0.0, 0.5, 2.0, 1.5,  1.5,  0.5, 0.5, 0.5, 2.0,
                     1.0,  1.0, 0.5, 0.5, 0.5,  0.25, 0.0, 0.0, 2.0, 1.0,
                     1.0,  0.5, 0.5, 0.5, 0.25, 0.0,  0.0};

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("attachmentViewer");
  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  if (argc != 4) {
    cerr << "Usage: attachmentViewer <meshfile.obj> <motionfile.bvh> "
         << "<weights.att>" << endl;
    exit(1);
  }
  setup(argv[1], argv[2], argv[3]);
  glutMainLoop();
  return 0;
}

void setup(char *objFileName, char *bvhFileName, char *attFileName) {
  // mesh
  obj.readObjFile(objFileName);
  obj.normalize();
  obj.createVertexArray();

  // skeleton
  skel.readBvhSkeleton(bvhFileName);
  skel.determineRadius();
  skel.recoverBones();

  // attachment
  att.distancesVisibility(boneRadii);
  att.readW(attFileName);

  // camera
  cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 200.0);
  cam.transl[2] = -1.0;

  // gl
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // camera view volume
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  cam.glVolume();

  // camera position
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cam.glPosition();

  // draw mesh
  obj.glPosition(Vector3f::Zero());
  att.glDrawMeshAttach(highlightMode, highlightBone);

  // draw skeleton
  skel.glDrawBonesHighlight(highlightBone);

  glutSwapBuffers();
}

void resize(int w, int h) { glViewport(0, 0, (GLsizei)w, (GLsizei)h); }

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;  // quit
  case 'w': // write
    obj.writeObjFile("output2.obj");
    skel.writeBvhSkeleton("output2.bvh");
    att.writeMatrixXi(att.V, "V2.out");
    att.writeMatrixXfSparse(att.W, "W2.out");
    break;
  case 'x': // reset
    obj.reset();
    cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);
    cam.transl[2] = -1.0;
    highlightBone = 0;
    highlightMode = false;
    break;
  case '+': // toggle through bones
    highlightBone = (highlightBone + 1) % skel.bones.size();
    break;
  case '-': // reverse toggle through bones
    highlightBone =
        (!highlightBone) ? skel.bones.size() - 1 : highlightBone - 1;
    break;
  case 'h':
    highlightMode = !highlightMode;
    break;
  default:
    break;
  }
  cam.keyInput(key); // camera controls

  glutPostRedisplay();
}
