#include <GL/glew.h>
#include <GL/freeglut.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "Model.hpp"
#include "ModelFactory.hpp"
#include "Camera.hpp"

// todo: move this somewhere else?
static const float PI = 3.14159265;
float degreesToRadians(float degrees) {
  return degrees * (PI / 180);
}
float radiansToDegrees(float radians) {
  return radians * (180 / PI);
}

Camera camera;
Model model;

// Display list identifier
static unsigned int aModel;

void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int key, int x, int y);
void setup(void);

void positionCamera(void);

int main(int argc, char** argv) {
  if (argc != 2) {
    throw std::runtime_error(
        "Incorrect arguments; one argument (path to model specifications) is "
        "expected");
  }

  ModelFactory modelFactory(argv[1]);
  model = modelFactory.getModel();

  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);

  glutCreateWindow(model.getName().c_str());

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

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  std::vector<float>& vertices = model.getVertices();
  std::vector<float>& colors = model.getColors();

  glVertexPointer(3, GL_FLOAT, 0, (float*)&vertices[0]);
  glColorPointer(3, GL_FLOAT, 0, (float*)&colors[0]);

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

  aModel = glGenLists(1);

  glNewList(aModel, GL_COMPILE);

  std::vector<std::vector<unsigned>>& polygons = model.getPolygons();
  for (std::vector<unsigned>& polygon : polygons) {
    glDrawElements(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_INT,
                   (unsigned*)&polygon[0]);
  }

  glEndList();
}

void drawScene(void) {
  positionCamera();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float fogColor[4] = {0.0, 0.0, 0.0, 0.0};

  glEnable(GL_FOG);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 10.0);
  glFogf(GL_FOG_END, 11.0);
  glFogf(GL_FOG_DENSITY, 0.01);
  glHint(GL_FOG_HINT, GL_NICEST);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glPushMatrix();

  // Translate model to position
  std::vector<float> displacement = model.getDisplacement();
  glTranslatef(displacement[0], displacement[1], displacement[2]);

  //// Extract angle and axis of rotation from quaternion, rotate model
  Eigen::Quaternion<float> modelOrientation = model.getOrientation();

  float angle = 0.0f, axisX = 0.0f, axisY = 0.0f, axisZ = 0.0f;
  angle = 2 * acos(modelOrientation.w());
  axisX = modelOrientation.x() / sin(angle / 2);
  axisY = modelOrientation.y() / sin(angle / 2);
  axisZ = modelOrientation.z() / sin(angle / 2);

  glRotatef(radiansToDegrees(angle), axisX, axisY, axisZ);

  // Scale model
  std::vector<float> modelScale = model.getScale();
  glScalef(modelScale[0], modelScale[1], modelScale[2]);

  // Translate model to origin
  auto modelCenter = model.getCenter();
  glTranslatef(-modelCenter[0], -modelCenter[1], -modelCenter[2]);

  glCallList(aModel);

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

  Camera::CAMERA_PROJECTION_MODE cameraProjectionMode =
      camera.getCameraProjectionMode();
  switch (cameraProjectionMode) {
    case Camera::CAMERA_PROJECTION_MODE::ORTHOGRAPHIC:
      glOrtho(-1.0, 1.0, -1.0, 1.0, 8.0, 100.0);
      break;
    case Camera::CAMERA_PROJECTION_MODE::PERSPECTIVE:
      glFrustum(-1.0, 1.0, -1.0, 1.0, 8.0, 100.0);
      break;
    default:
      throw std::runtime_error("Unrecognized camera projection mode");
  }

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
      Eigen::Quaternion<float> modelOrientation = model.getOrientation();
      model.rotate(modelOrientation.inverse());

      std::vector<float> modelPosition = model.getDisplacement();
      model.translate(std::vector<float>{-modelPosition[0], -modelPosition[1],
                                         -modelPosition[2] - 10.0f});

      Eigen::Quaternion<float> cameraOrientation = camera.getOrientation();
      camera.rotate(cameraOrientation.inverse());

      std::vector<float> cameraPosition = camera.getDisplacement();
      camera.translate(std::vector<float>{
          -cameraPosition[0], -cameraPosition[1], -cameraPosition[2]});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'w':
      model.writeToFile("out.obj");
      break;
    case 'v': {
      camera.setCameraProjectionMode(
          Camera::CAMERA_PROJECTION_MODE::ORTHOGRAPHIC);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'V': {
      camera.setCameraProjectionMode(
          Camera::CAMERA_PROJECTION_MODE::PERSPECTIVE);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'n': {
      model.translate(std::vector<float>{0.0, 0.0, -0.1});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'N': {
      model.translate(std::vector<float>{0.0, 0.0, 0.1});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'p': {
      float rotationAngle = degreesToRadians(-10);

      Eigen::Quaternion<float> rotationDelta(
          cos(rotationAngle / 2), sin(rotationAngle / 2), 0.0f, 0.0f);

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'P': {
      float rotationAngle = degreesToRadians(10);

      Eigen::Quaternion<float> rotationDelta(
          cos(rotationAngle / 2), sin(rotationAngle / 2), 0.0f, 0.0f);

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'y': {
      float rotationAngle = degreesToRadians(-10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                             sin(rotationAngle / 2), 0.0f);

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'Y': {
      float rotationAngle = degreesToRadians(10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                             sin(rotationAngle / 2), 0.0f);

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'r': {
      float rotationAngle = degreesToRadians(-10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                             sin(rotationAngle / 2));

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'R': {
      float rotationAngle = degreesToRadians(10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                             sin(rotationAngle / 2));

      model.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'd': {
      camera.translate(std::vector<float>{-0.1f, 0.0f, 0.0f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'D': {
      camera.translate(std::vector<float>{0.1f, 0.0f, 0.0f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'c': {
      camera.translate(std::vector<float>{0.0f, -0.1f, 0.0f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'C': {
      camera.translate(std::vector<float>{0.0f, 0.1f, 0.0f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'z': {
      camera.translate(std::vector<float>{0.0f, 0.0f, -0.1f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'Z': {
      camera.translate(std::vector<float>{0.0f, 0.0f, 0.1f});

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 't': {
      float rotationAngle = degreesToRadians(-1);

      Eigen::Quaternion<float> rotationDelta(
          cos(rotationAngle / 2), sin(rotationAngle / 2), 0.0f, 0.0f);
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'T': {
      float rotationAngle = degreesToRadians(1);

      Eigen::Quaternion<float> rotationDelta(
          cos(rotationAngle / 2), sin(rotationAngle / 2), 0.0f, 0.0f);
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'a': {
      float rotationAngle = degreesToRadians(-1);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                             sin(rotationAngle / 2), 0.0f);
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'A': {
      float rotationAngle = degreesToRadians(1);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f,
                                             sin(rotationAngle / 2), 0.0f);
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'l': {
      float rotationAngle = degreesToRadians(-10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                             sin(rotationAngle / 2));
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    case 'L': {
      float rotationAngle = degreesToRadians(10);

      Eigen::Quaternion<float> rotationDelta(cos(rotationAngle / 2), 0.0f, 0.0f,
                                             sin(rotationAngle / 2));
      camera.rotate(rotationDelta);

      glutPostRedisplay();  // re-draw scene
      break;
    }
    default:
      break;
  }
}

void specialKeyInput(int key, int x, int y) {
  if (key == GLUT_KEY_UP)
    model.translate(std::vector<float>{0.0, 0.1, 0.0});
  if (key == GLUT_KEY_DOWN)
    model.translate(std::vector<float>{0.0, -0.1, 0.0});
  if (key == GLUT_KEY_LEFT)
    model.translate(std::vector<float>{-0.1, 0.0, 0.0});
  if (key == GLUT_KEY_RIGHT)
    model.translate(std::vector<float>{0.1, 0.0, 0.0});

  glutPostRedisplay();  // re-draw scene
}