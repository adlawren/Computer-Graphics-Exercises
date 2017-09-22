#include <GL/glew.h>
#include <GL/freeglut.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "ModelData.hpp"
#include "Camera.hpp"

Camera camera;
ModelData modelData;

// Display list identifier
static unsigned int aModel;

void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void setup(void);
ModelData loadModelSpecification(std::ifstream& fileStream);
void positionCamera(void);

int main(int argc, char** argv) {
  if (argc != 2) {
    throw std::runtime_error(
        "Incorrect arguments; one argument (path to model specifications) is "
        "expected");
  }

  // Load the data from the file into a global data structure
  std::ifstream modelSpecificationFileStream(argv[1]);
  modelData = loadModelSpecification(modelSpecificationFileStream);

  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);

  glutCreateWindow(modelData.getName().c_str());

  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);

  // todo: needed?
  glewExperimental = GL_TRUE;
  glewInit();

  setup();

  glutMainLoop();
}

ModelData loadModelSpecification(std::ifstream& fileStream) {
  ModelData modelData;

  if (!fileStream.good()) {
    throw std::runtime_error(
        "The given model specification file path is invalid");
  }

  const std::string fileFormatErrorMessage =
      "invalid model specification file format";

  unsigned currentLineNumber = 0, firstPolygonLineNumber = -1;
  std::string nextLine;
  while (std::getline(fileStream, nextLine)) {
    ++currentLineNumber;

    std::stringstream ss(nextLine);
    char c = nextLine[0];
    switch (c) {
      case 'o': {
        if (currentLineNumber != 1)
          throw std::runtime_error(fileFormatErrorMessage);

        modelData.setName(&nextLine[2]);
        break;
      }
      case 'v': {
        if (currentLineNumber > firstPolygonLineNumber)
          throw std::runtime_error(fileFormatErrorMessage);

        std::stringstream ss(nextLine);
        char c;
        float f1, f2, f3;
        if (!(ss >> c >> f1 >> f2 >> f3)) {
          throw std::runtime_error(fileFormatErrorMessage);
        }

        modelData.addVertex(std::vector<float>{f1, f2, f3});
        break;
      }
      case 'f': {
        if (firstPolygonLineNumber == -1)
          firstPolygonLineNumber = currentLineNumber;

        std::stringstream ss(nextLine);
        char c;
        unsigned u1, u2, u3, u4;
        if (!(ss >> c >> u1 >> u2 >> u3 >> u4)) {
          throw std::runtime_error(fileFormatErrorMessage);
        }

        // Subtract by 1; the data is 1-indexed
        modelData.addPolygon(
            std::vector<unsigned>{u1 - 1, u2 - 1, u3 - 1, u4 - 1});
        break;
      }
      default:
        throw std::runtime_error(fileFormatErrorMessage);
    }
  }

  return modelData;
}

void setup(void) {
  // Translate model to origin
  std::vector<float> modelCenter = modelData.getCenter();
  modelData.translate(
      std::vector<float>{-modelCenter[0], -modelCenter[1], -modelCenter[2]});

  // Scale the model
  std::vector<float> modelDimensions = modelData.getDimensions();
  float maxDimension = std::max(
      std::max(modelDimensions[0], modelDimensions[1]), modelDimensions[2]);

  modelData.scale(
      std::vector<float>{1 / maxDimension, 1 / maxDimension, 1 / maxDimension});
  modelData.scale(std::vector<float>{1.25, 1.25, 1.25});

  // Translate model to (0, 0, -10)
  // modelData.translate(std::vector<float>{0, 0, -10}); // todo: rm?

  aModel = glGenLists(1);

  glNewList(aModel, GL_COMPILE);

  std::vector<std::vector<float>> vertices = modelData.getVertices();

  for (std::vector<unsigned> polygon : modelData.getPolygons()) {
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

  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void drawScene(void) {
  positionCamera();

  glClear(GL_COLOR_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glPushMatrix();

  // todo: fix this static translate; do something else
  glTranslatef(0.0, 0.0, -10.0);

  std::vector<float> scale = modelData.getScale();
  glScalef(scale[0], scale[1], scale[2]);

  std::vector<float> displacement = modelData.getDisplacement();
  glTranslatef(displacement[0], displacement[1], displacement[2]);

  glCallList(aModel);

  glPopMatrix();

  glFlush();
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
    // Escape-key callback
    case 'q':
      exit(0);
      break;
    case 'w':
      modelData.writeModelFile("out.obj");
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
    default:
      break;
  }
}
