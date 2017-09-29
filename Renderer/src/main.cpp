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
  model = modelFactory.getNormalizedModel();

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

  glEnable(GL_DEPTH_TEST);

  // Translate model to (0, 0, -10)
  model.translate(std::vector<float>{0, 0, -10});

  aModel = glGenLists(1);

  glNewList(aModel, GL_COMPILE);

  std::vector<std::vector<float>> vertices = model.getVertices();

  for (std::vector<unsigned> polygon : model.getPolygons()) {
    // todo: check 'polygon' size; it could be a triangle, not a 'polygon'
    glBegin(GL_POLYGON);

    // todo: use model colors
    glColor3f(1.0, 1.0, 1.0);

    for (unsigned vertexIndex : polygon) {
      glVertex3f(vertices[vertexIndex][0], vertices[vertexIndex][1],
                 vertices[vertexIndex][2]);
    }

    glEnd();
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

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
      exit(0);
      break;
    case 'x': {
      std::vector<float> modelPosition = model.getDisplacement();

      model.translate(std::vector<float>{-modelPosition[0], -modelPosition[1],
                                         -modelPosition[2] - 10.0f});

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