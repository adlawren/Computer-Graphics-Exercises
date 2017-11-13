// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

// motionViewer <motion.bvh>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "skeleton.h"
#include "motion.h"
#include "timer.h"
#include "camera.h"

int main(int, char **);
void setup(char *);
void drawScene(void);
void idle(void);
void resize(int, int);
void keyInput(unsigned char, int, int);

/* global variables */
camera cam;
skeleton skel;
motion mot;
timer tim;
bool interpolate(true);

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("motionViewer");
  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  glutIdleFunc(idle);
  if (argc != 2) {
    cerr << "Usage: motionViewer <motionfile.bvh>" << endl;
    exit(1);
  }
  setup(argv[1]);
  glutMainLoop();
  return 0;
}

void setup(char *fileName) {
  // skeleton
  skel.readBvhSkeleton(fileName);
  skel.determineRadius();

  // motion
  mot.readBvhMotion(fileName, skel);
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

  // gl
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  // camera
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cam.glPosition();

  // skeleton
  skel.glColor();
  skel.interpolatePose(&mot, tim.loopFrac, interpolate);
  skel.glDraw();

  glutSwapBuffers();
}

void idle(void) { tim.glIdle(); }

void resize(int w, int h) { glViewport(0, 0, (GLsizei)w, (GLsizei)h); }

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;  // quit
  case 'w': // write bvh file
    skel.writeBvhSkeleton("output.bvh");
    mot.writeBvhMotion("output.bvh");
    break;
  case 'x': // reset
    tim.initialize(true, mot.sequence.size(), mot.defaultGapTime);
    cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 200.0);
    cam.positionMotion(mot.range, skel.radius);
    break;
  default:
    break;
  }
  cam.keyInput(key); // camera controls
  tim.keyInput(key); // timer controls

  glutPostRedisplay();
}
