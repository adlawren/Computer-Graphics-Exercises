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
#include "Model.hpp"        // todo: rm
#include "ModelFactory.hpp" // todo: rm
#include "SkeletonFactory.hpp"

// todo: move this somewhere else?
static const float PI = 3.14159265;
float degreesToRadians(float degrees) { return degrees * (PI / 180); }
float radiansToDegrees(float radians) { return radians * (180 / PI); }

Camera camera;
Model model; // todo: rm
Skeleton skeleton;

// Display list identifier
static unsigned int aSkeleton;

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

  // return 0; // todo: rm

  ModelFactory modelFactory(argv[1]);
  model = modelFactory.getModel();

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

  // glEnableClientState(GL_VERTEX_ARRAY);
  // glEnableClientState(GL_COLOR_ARRAY);

  std::vector<float> &vertices = model.getVertices();
  std::vector<float> &colors = model.getColors();

  // glVertexPointer(3, GL_FLOAT, 0, (float *)&vertices[0]);
  // glColorPointer(3, GL_FLOAT, 0, (float *)&colors[0]);

  glEnable(GL_DEPTH_TEST);

  // Move the model into the viewing frustum
  model.translate(std::vector<float>{0, 0, -10});

  //// Scale the model to fit within screen
  std::vector<float> modelDimensions = model.getDimensions();
  float maxDimension = std::max(
      std::max(modelDimensions[0], modelDimensions[1]), modelDimensions[2]);

  std::vector<float> scale =
      std::vector<float>{1 / maxDimension, 1 / maxDimension, 1 / maxDimension};
  scale[0] *= 1.25;
  scale[1] *= 1.25;
  scale[2] *= 1.25;

  model.scale(scale);

  aSkeleton = glGenLists(1);

  glNewList(aSkeleton, GL_COMPILE);

  std::vector<std::vector<unsigned>> &polygons = model.getPolygons();
  for (std::vector<unsigned> &polygon : polygons) {
    glDrawElements(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_INT,
                   (unsigned *)&polygon[0]);
  }

  glEndList();
}

// recursive method to render sucessive nodes depth-first (?)
void renderSkeleton(SkeletonTree::Node *node) {
  glPushMatrix();

  SkeletonTree::Node::Offset nodeOffset = node->getOffset();

  // todo: apply rotation matrix
  // ...

  // render line
  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(nodeOffset[0], nodeOffset[1], nodeOffset[2]);
  glEnd();

  // apply translation matrix
  glTranslatef(nodeOffset[0], nodeOffset[1], nodeOffset[2]);

  // render children
  for (SkeletonTree::Node *nextChildNode : node->getChildNodes()) {
    renderSkeleton(nextChildNode);
  }

  glPopMatrix();
}

void drawScene(void) {
  positionCamera();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // float fogColor[4] = {0.0, 0.0, 0.0, 0.0};
  //
  // glEnable(GL_FOG);
  // glFogfv(GL_FOG_COLOR, fogColor);
  // glFogi(GL_FOG_MODE, GL_LINEAR);
  // glFogf(GL_FOG_START, 10.0);
  // glFogf(GL_FOG_END, 11.0);
  // glFogf(GL_FOG_DENSITY, 0.01);
  // glHint(GL_FOG_HINT, GL_NICEST);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glColor3f(1.0, 1.0, 1.0);

  glPushMatrix();

  // translate skeleton into field of view
  glTranslatef(0.0f, 0.0f, -10.0f);

  // todo: see if you can use the depth first search method of the SkeletonTree
  // class instead...
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
  glFrustum(-20.0, 20.0, -20.0, 20.0, 8.0, 100.0);

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

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;
  // TODO: MAKE SURE THAT YOU UPDATE THIS; RESET EVERYTHING
  case 'x': {
    Eigen::Quaternion<float> cameraOrientation = camera.getOrientation();
    camera.rotate(cameraOrientation.inverse());

    std::vector<float> cameraPosition = camera.getDisplacement();
    camera.translate(std::vector<float>{-cameraPosition[0], -cameraPosition[1],
                                        -cameraPosition[2]});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'w':
    skeleton.writeToFile("output.bvh");
    break;
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
