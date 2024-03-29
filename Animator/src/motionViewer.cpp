#include <GL/glew.h>
#include <GL/freeglut.h>

#include <exception>

#include "Camera.hpp"
#include "SkeletonFactory.hpp"

#include "geometry.hpp"

Camera camera;
Skeleton skeleton;

//// animaion parameters
bool isAnimate = false;

void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
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
  Eigen::Quaternion<float> jointRotation = node->getRotationQuaternion();

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

  //// apply user-specified transformations
  //// extract angle and axis of rotation from quaternion, rotate camera
  Eigen::Quaternion<float> cameraOrientation = camera.getOrientation();

  float angle = 0.0f, axisX = 0.0f, axisY = 0.0f, axisZ = 0.0f;
  angle = 2 * acos(cameraOrientation.w());
  axisX = cameraOrientation.x() / sin(angle / 2);
  axisY = cameraOrientation.y() / sin(angle / 2);
  axisZ = cameraOrientation.z() / sin(angle / 2);

  // note: angle needs to be in degrees
  glRotatef(radiansToDegrees(angle), axisX, axisY, axisZ);

  std::vector<float> cameraDisplacement = camera.getDisplacement();
  glTranslatef(-cameraDisplacement[0], -cameraDisplacement[1],
               -cameraDisplacement[2]);

  //// focus camera on animation
  std::array<float, 6> animationDimensionBounds =
      skeleton.getAnimationDimensionBounds();

  // scale dimensions
  std::vector<float> dimensionScale = {1.0f, 1.0f, 1.0f};
  animationDimensionBounds[0] *= dimensionScale[0];
  animationDimensionBounds[1] *= dimensionScale[0];
  animationDimensionBounds[2] *= dimensionScale[1];
  animationDimensionBounds[3] *= dimensionScale[1];
  animationDimensionBounds[4] *= dimensionScale[2];
  animationDimensionBounds[5] *= dimensionScale[2];

  float frustumWidth =
      animationDimensionBounds[1] - animationDimensionBounds[0];
  float frustumHeight =
      animationDimensionBounds[3] - animationDimensionBounds[2];
  float frustumDepth =
      animationDimensionBounds[5] - animationDimensionBounds[4];

  glFrustum(-frustumWidth / 2, frustumWidth / 2, -frustumHeight / 2,
            frustumHeight / 2, 40.0f, 200.0f);

  float cameraZOffset =
      -(animationDimensionBounds[4] + frustumDepth / 2) - 100.0f;

  glTranslatef(-(animationDimensionBounds[0] + frustumWidth / 2),
               -(animationDimensionBounds[2] + frustumHeight / 2),
               cameraZOffset);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void animate() {
  if (skeleton.isPaused()) {
    skeleton.unpauseAnimation();
  }

  skeleton.applyNextFrame();

  glutPostRedisplay();
}

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break;
  // TODO: MAKE SURE THAT YOU UPDATE THIS; RESET EVERYTHING
  case 'x': {
    isAnimate = false;
    glutIdleFunc(NULL);

    skeleton.reset();
    camera.reset();

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'w':
    skeleton.writeToFile("output.bvh");
    break;
  case 'p': {
    if (!isAnimate) {
      isAnimate = true;
      glutIdleFunc(animate);
    }

    break;
  }
  case 'P': {
    isAnimate = false;
    glutIdleFunc(NULL);
    skeleton.pauseAnimation();
    break;
  }
  case 'd': {
    camera.translate(std::vector<float>{-1.0f, 0.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'D': {
    camera.translate(std::vector<float>{1.0f, 0.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'c': {
    camera.translate(std::vector<float>{0.0f, -1.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'C': {
    camera.translate(std::vector<float>{0.0f, 1.0f, 0.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'z': {
    camera.translate(std::vector<float>{0.0f, 0.0f, -1.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'Z': {
    camera.translate(std::vector<float>{0.0f, 0.0f, 1.0f});

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 't': {
    float rotationAngle = degreesToRadians(-10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2),
                                           sin(rotationAngle / 2), 0.0f, 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'T': {
    float rotationAngle = degreesToRadians(10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2),
                                           sin(rotationAngle / 2), 0.0f, 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'a': {
    float rotationAngle = degreesToRadians(-10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                           sin(rotationAngle / 2), 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'A': {
    float rotationAngle = degreesToRadians(10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                           sin(rotationAngle / 2), 0.0f);
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  // note: the z-rotation directions have been reversed to coincide with
  // expected rotation directions. Not sure why this is needed...
  case 'l': {
    float rotationAngle = degreesToRadians(10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                           sin(rotationAngle / 2));
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case 'L': {
    float rotationAngle = degreesToRadians(-10);

    Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                           sin(rotationAngle / 2));
    camera.rotate(rotationDelta);

    glutPostRedisplay(); // re-draw scene
    break;
  }
  case '+': {
    skeleton.updateFPS(10);
    break;
  }
  case '-': {
    skeleton.updateFPS(-10);
    break;
  }
  default:
    break;
  }
}
