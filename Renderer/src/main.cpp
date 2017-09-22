#include <GL/glew.h>
#include <GL/freeglut.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

// todo: move to seperate file
class ModelData {
 public:
  ModelData() {
  }

  ModelData(const std::string& modelName,
            const std::vector<std::vector<float>>& vertices,
            const std::vector<std::vector<unsigned>>& polygons)
      : modelName_(modelName), vertices_(vertices), polygons_(polygons) {
  }

  std::string getName() const {
    return modelName_;
  }
  void setName(const std::string& newModelName) {
    modelName_ = newModelName;
  }

  std::vector<std::vector<float>> getVertices() const {
    return vertices_;
  }
  void addVertex(const std::vector<float> newVertex) {
    if (newVertex.size() != 3)
      throw std::runtime_error("Vertices must contain 3 numbers");

    vertices_.push_back(newVertex);

    // todo: do better
    // ... really, vertices should be a list of pairs;
    // ... coupling the vertex position and colors
    for (int i = 0; i < 3; ++i) {
      colors_.push_back(0.0f);
    }
  }

  void printVertices() const {
    for (auto vertex : vertices_) {
      std::cout << std::endl << "v ";
      for (float vertexValue : vertex) {
        std::cout << vertexValue << " ";
      }
    }

    std::cout << std::endl;
  }

  std::vector<float> getColors() const {
    return colors_;
  }

  std::vector<std::vector<unsigned>> getPolygons() {
    return polygons_;
  }

  void addPolygon(const std::vector<unsigned>& newPolygon) {
    for (unsigned vertexIndex : newPolygon) {
      if (vertexIndex >= vertices_.size()) {
        throw std::runtime_error("Invalid vertex index specified");
      }
    }

    polygons_.push_back(newPolygon);
  }

  void printPolygons() const {
    for (auto polygon : polygons_) {
      std::cout << "f ";

      for (auto vertexIndex : polygon) {
        std::cout << vertexIndex << " ";
      }

      std::cout << std::endl;
    }
  }

 private:
  std::string modelName_;
  std::vector<std::vector<float>> vertices_;
  std::vector<float> colors_;  // todo: do better
  std::vector<std::vector<unsigned>> polygons_;
};

ModelData modelData;

void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void setup(void);
ModelData loadModelSpecification(std::ifstream& fileStream);

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
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glColor3f(1.0, 1.0, 1.0);

  std::vector<std::vector<float>> vertices = modelData.getVertices();

  // todo: iterate over the global data structure, draw the polygons
  for (std::vector<unsigned> polygon : modelData.getPolygons()) {
    static unsigned* test3 = &polygon[0];
    glBegin(GL_POLYGON);

    // unsigned vertexOneIndex = test3[0] * 3;
    // glVertex3f(vertices[vertexOneIndex], vertices[vertexOneIndex + 1],
    //            vertices[vertexOneIndex + 2]);

    // unsigned vertexTwoIndex = test3[1] * 3;
    // glVertex3f(vertices[vertexTwoIndex], vertices[vertexTwoIndex + 1],
    //            vertices[vertexTwoIndex + 2]);

    // unsigned vertexThreeIndex = test3[2] * 3;
    // glVertex3f(vertices[vertexThreeIndex], vertices[vertexThreeIndex + 1],
    //            vertices[vertexThreeIndex + 2]);

    // unsigned vertexFourIndex = test3[3] * 3;
    // glVertex3f(vertices[vertexFourIndex], vertices[vertexFourIndex + 1],
    //            vertices[vertexFourIndex + 2]);

    for (unsigned vertexIndex : polygon) {
      glVertex3f(vertices[vertexIndex][0], vertices[vertexIndex][1],
                 vertices[vertexIndex][2]);
    }

    glEnd();
  }

  glFlush();
}

void resize(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);
  glOrtho(-5.0, 5.0, -5.0, 5.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
    // Escape-key callback
    case 27:
      exit(0);
      break;
    default:
      break;
  }
}
