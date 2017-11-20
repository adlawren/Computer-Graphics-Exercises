// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

// personViewer <mesh.obj> <motion.bvh> <weights.att>

#include <Eigen/Dense>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "camera.h"
#include "mesh.h"
#include "skeleton.h"
#include "attachment.h"
#include "light.h"
#include "texture.h"

using namespace Eigen;

int main(int, char **);
void setup(char *, char *, char *);
void drawScene(void);
void idle(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int, int, int);

/* global variables */
camera cam;
mesh obj;
motion mot;
skeleton skel;
timer tim;
bool interpolate(true);
attachment att(&obj, &skel);
float boneRadii[] = {0.5,  2.0, 1.0, 0.5, 0.25, 0.0,  0.5, 2.0, 1.0, 0.5,
                     0.25, 0.0, 0.5, 2.0, 1.5,  1.5,  0.5, 0.5, 0.5, 2.0,
                     1.0,  1.0, 0.5, 0.5, 0.5,  0.25, 0.0, 0.0, 2.0, 1.0,
                     1.0,  0.5, 0.5, 0.5, 0.25, 0.0,  0.0};
bool modelWasReset = true;

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("personViewer");
  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  glutIdleFunc(idle);
  if (argc != 4) {
    cerr << "Usage: personViewer <meshfile.obj> <motionfile.bvh> "
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

  // skeleton
  skel.readBvhSkeleton(bvhFileName);
  skel.determineRadius();
  skel.recoverBones();

  // attachment
  att.distancesVisibility(boneRadii);
  att.readW(attFileName);

  // motion
  mot.readBvhMotion(bvhFileName, skel);
  mot.determineRange(skel.translationIndices);

  // timer
  tim.initialize(true, mot.sequence.size(), mot.defaultGapTime);

  // camera
  cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 200.0);
  cam.positionMotion(mot.range, skel.radius);

  // camera view volume
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  cam.glVolume();
  cam.glPosition();

  // gl
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);

  // Configure lighting
  light l;
  l.glConfigureLighting();

  material *meshMaterial = obj.getMaterial();
  meshMaterial->glConfigureMaterialProperties();

  // Load skin texture
  texture meshTexture(meshMaterial);
  meshTexture.glLoadTexture();
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

  cam.glConfigureDisplayMode();

  //// interpolate skeleton pose
  // display the null pose when the model is reset
  if (modelWasReset) {
    skel.nullPose();
  } else {
    skel.interpolatePose(&mot, tim.loopFrac, interpolate);
  }

  // draw deformed mesh
  att.glDrawDeformedMesh();

  glutSwapBuffers();
}

void idle(void) { tim.glIdle(); }

void resize(int w, int h) { glViewport(0, 0, (GLsizei)w, (GLsizei)h); }

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;  // quit
  case 'w': // write
    obj.writeObjFile("meshout.obj");
    skel.writeBvhSkeleton("skeletonout.bvh");
    mot.writeBvhMotion("skeletonout.bvh");
    att.writeMatrixXfSparse(att.W, "attachout.att");
    break;
  // TODO: MAKE SURE THAT THIS IS UP TO DATE; RESET EVERYTHING
  case 'x':      // reset
    obj.reset(); // ???

    tim.initialize(true, mot.sequence.size(), mot.defaultGapTime);
    cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 200.0);
    cam.positionMotion(mot.range, skel.radius);

    modelWasReset = true;
    break;
  case 'p':
    modelWasReset = false;
    break;
  default:
    break;
  }
  cam.keyInput(key); // camera controls
  tim.keyInput(key); // timer controls

  glutPostRedisplay();
}
