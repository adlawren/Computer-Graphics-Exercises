#include <GL/glew.h>
#include <GL/freeglut.h>

// todo: confirm whether or not all of these are needed
#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "Camera.hpp"
#include "SkeletonFactory.hpp"

// todo: move this somewhere else?
static const float PI = 3.14159265;
float degreesToRadians(float degrees) { return degrees * (PI / 180); }
float radiansToDegrees(float radians) { return radians * (180 / PI); }

Camera camera;
Skeleton skeleton;

// bool to toggle animation
bool isAnimate = false;

void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int key, int x, int y);
void setup(void);

void positionCamera(void);

int main(int argc, char **argv) {
  if (argc != 2) {
    throw std::runtime_error("Incorrect arguments; one argument (path to "
                             "motion capture specifications) is "
                             "expected");
  }

  SkeletonFactory skeletonFactory(argv[1]);
  skeleton = skeletonFactory.getSkeleton();

  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);

  glutCreateWindow(argv[1]);

  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);

  glutSpecialFunc(specialKeyInput);

  // todo: needed?
  glewExperimental = GL_TRUE;
  glewInit();

  setup();

  glutMainLoop();
}

void setup(void) {
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glEnable(GL_DEPTH_TEST);
}

// recursive method to render sucessive nodes depth-first
void renderSkeleton(SkeletonTree::Node *node, bool isRoot = true) {
  glPushMatrix();

  //// render line
  SkeletonTree::Node::Offset nodeOffset = node->getOffset();

  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(nodeOffset[0], nodeOffset[1], nodeOffset[2]);
  glEnd();

  if (isRoot) {
    // apply root translation
    SkeletonTree::Node::Channel translationChannel =
        node->getTranslationChannel();

    glTranslatef(translationChannel[0], translationChannel[1],
                 translationChannel[2]);
  }

  // apply joint offset
  glTranslatef(nodeOffset[0], nodeOffset[1], nodeOffset[2]);

  //// apply joint rotation
  SkeletonTree::Node::Channel rotationChannel = node->getAngleChannel();

  Eigen::Quaternion<float> jointRotation = Eigen::Quaternion<float>::Identity();

  Eigen::Quaternion<float> zAxisRotation(
      cos(degreesToRadians(rotationChannel[0]) / 2), 0.0f, 0.0f,
      sin(degreesToRadians(rotationChannel[0]) / 2));
  Eigen::Quaternion<float> yAxisRotation(
      cos(degreesToRadians(rotationChannel[1]) / 2), 0.0f,
      sin(degreesToRadians(rotationChannel[1]) / 2), 0.0f);
  Eigen::Quaternion<float> xAxisRotation(
      cos(degreesToRadians(rotationChannel[2]) / 2),
      sin(degreesToRadians(rotationChannel[2]) / 2), 0.0f, 0.0f);

  jointRotation = zAxisRotation * yAxisRotation * xAxisRotation * jointRotation;
  jointRotation.normalize();

  float angle = 0.0f, axisX = 0.0f, axisY = 0.0f, axisZ = 0.0f;
  angle = 2 * acos(jointRotation.w());
  axisX = jointRotation.x() / sin(angle / 2);
  axisY = jointRotation.y() / sin(angle / 2);
  axisZ = jointRotation.z() / sin(angle / 2);

  // note: angle needs to be in degrees
  glRotatef(radiansToDegrees(angle), axisX, axisY, axisZ);

  for (SkeletonTree::Node *nextChildNode : node->getChildNodes()) {
    renderSkeleton(nextChildNode, false);
  }

  glPopMatrix();
}

void drawScene(void) {
  positionCamera();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glColor3f(1.0, 1.0, 1.0);

  glPushMatrix();

  // translate skeleton into field of view
  // glTranslatef(0.0f, 0.0f, -30.0f);
  // glTranslatef(0.0f, 0.0f, -10.0f);
  // glTranslatef(0.0f, 0.0f, -1.0f);
  // glTranslatef(-10.0f, 10.0f, -10.0f); // ***
  glTranslatef(0.0f, 0.0f, -30.0f);

  // render skeleton joints
  renderSkeleton(skeleton.getSkeletonTree().getRootNode());

  glPopMatrix();

  glutSwapBuffers();
}

void resize(int w, int h) {
  glViewport(0, 0, w, h);
  positionCamera();
}

void positionCamera(void) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // use perspective mode exclusively
  // glFrustum(-5.0, 5.0, -5.0, 5.0, 1.0, 100.0);
  // glFrustum(-20.0, 20.0, -20.0, 20.0, 0.5, 100.0);
  // glFrustum(-30.0, 30.0, 30.0, 30.0, 8.0, 100.0);
  glOrtho(-100.0, 100.0, -100.0, 100.0, 0.5, 100.0);

  std::vector<float> cameraDisplacement = camera.getDisplacement();
  glTranslatef(cameraDisplacement[0], cameraDisplacement[1],
               cameraDisplacement[2]);

  //// Extract angle and axis of rotation from quaternion, rotate camera
  Eigen::Quaternion<float> cameraOrientation = camera.getOrientation();

  float angle = 0.0f, axisX = 0.0f, axisY = 0.0f, axisZ = 0.0f;
  angle = 2 * acos(cameraOrientation.w());
  axisX = cameraOrientation.x() / sin(angle / 2);
  axisY = cameraOrientation.y() / sin(angle / 2);
  axisZ = cameraOrientation.z() / sin(angle / 2);

  // note: angle needs to be in degrees
  glRotatef(radiansToDegrees(angle), axisX, axisY, axisZ);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void animate(int val) {
  if (isAnimate) {
    // get next animation frame
    skeleton.applyNextFrame();

    glutPostRedisplay();
    glutTimerFunc(8, animate, 1);
  }
}

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;
  // TODO: MAKE SURE THAT YOU UPDATE THIS; RESET EVERYTHING
  case 'x': {
    isAnimate = false;

    skeleton.reset();
    camera.reset();

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'w':
    skeleton.writeToFile("output.bvh");
    break;
  case 'p': {
    isAnimate = true;
    animate(1);

    break;
  }
  case 'P': {
    isAnimate = false;
    break;
  }
  case 'd': {
    camera.translate(std::vector<float>{-0.1f, 0.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'D': {
    camera.translate(std::vector<float>{0.1f, 0.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'c': {
    camera.translate(std::vector<float>{0.0f, -0.1f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'C': {
    camera.translate(std::vector<float>{0.0f, 0.1f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'z': {
    camera.translate(std::vector<float>{0.0f, 0.0f, -0.1f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'Z': {
    camera.translate(std::vector<float>{0.0f, 0.0f, 0.1f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 't': {
    float rotationAngle = degreesToRadians(-1);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2),
                                           sin(rotationAngle / 2), 0.0f, 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'T': {
    float rotationAngle = degreesToRadians(1);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2),
                                           sin(rotationAngle / 2), 0.0f, 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'a': {
    float rotationAngle = degreesToRadians(-1);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                           sin(rotationAngle / 2), 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'A': {
    float rotationAngle = degreesToRadians(1);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                           sin(rotationAngle / 2), 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'l': {
    float rotationAngle = degreesToRadians(-10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                           sin(rotationAngle / 2));
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'L': {
    float rotationAngle = degreesToRadians(10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                           sin(rotationAngle / 2));
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  default:
    break;
  }
}

void specialKeyInput(int key, int x, int y) {
  // todo?

  glutPostRedisplay(); // re-draw scene
}
