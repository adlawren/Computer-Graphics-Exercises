#include <GL/glew.h>
#include <GL/freeglut.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

// todo: rm, testing
static float vertices2[] = {20.0f, 20.0f, 0.0f, 80.0f, 20.0f, 0.0f,
                            80.0f, 80.0f, 0.0f, 20.0f, 80.0f, 0.0f};
static float colors2[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
static unsigned polygon2[] = {0, 1, 2, 3};

class ModelData {
 public:
  ModelData() {
  }

  ModelData(const std::string& modelName, const std::vector<float>& vertices,
            const std::vector<std::vector<unsigned>>& polygons)
      : modelName_(modelName), vertices_(vertices), polygons_(polygons) {
  }

  std::string getName() const {
    return modelName_;
  }
  void setName(const std::string& newModelName) {
    modelName_ = newModelName;
  }

  std::vector<float> getVertices() const {
    return vertices_;
  }
  void addVertex(const std::vector<float> newVertex) {
    if (newVertex.size() != 3)
      throw std::runtime_error("Vertices must contain 3 numbers");

    for (float vertexValue : newVertex) {
      vertices_.push_back(vertexValue);
    }

    // todo: do better
    for (int i = 0; i < 3; ++i) {
      colors_.push_back(0.0f);
    }
  }

  // todo: rm?
  void printVertices() const {
    unsigned vertexValueIndex = 0;
    for (auto vertexValue : vertices_) {
      if (vertexValueIndex % 3 == 0) {
        std::cout << std::endl << "v ";
      }

      std::cout << vertexValue << " ";

      vertexValueIndex += 1;
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
    // todo: assert that the new polygon only contains vaid index values
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
  std::vector<float> vertices_;
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

  // todo: rm
  // modelData.printVertices();
  // modelData.printPolygons();

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
  glClearColor(1.0, 1.0, 1.0, 0.0);
  // glClearColor(0.0, 0.0, 0.0, 0.0); // todo: rm

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  std::vector<float> vertices = modelData.getVertices();
  // static float* test1 = vertices2;  //&vertices[0];

  // todo: this is the issue.
  // ... the array needs to be statically, stack allocated; size needs to be
  // ... known beforehand.
  // proposal: pre-allocate an array with a fixed buffer size
  // ... later, use numerical constraints to ensure that only the subset of
  // ... interest is used. can do a check when parsing the model to verify
  // ... that the number of vertices does not exceed the buffer size
  static float test_again1[12];  // [vertices.size()]; -> doesn't work
  for (int i = 0; i < vertices.size(); ++i) {
    test_again1[i] = vertices[i];
  }

  // static float* test1 = (float*)&vertices[0];
  static float* test1 = test_again1;

  // glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
  // glVertexPointer(3, GL_FLOAT, 0, vertices2);
  glVertexPointer(3, GL_FLOAT, 0, test1);

  // float* test1 = &vertices[0];
  for (int i = 0; i < 12; ++i) {
    std::cout << test1[i] << " ";
  }
  std::cout << std::endl;

  std::vector<float> colors = modelData.getColors();
  static float* test2 = colors2;  //&colors[0];
  // glColorPointer(3, GL_FLOAT, 0, &colors[0]);
  // glColorPointer(3, GL_FLOAT, 0, colors2);
  glColorPointer(3, GL_FLOAT, 0, test2);

  // float* test2 = &colors[0];
  for (int i = 0; i < 12; ++i) {
    std::cout << test2[i] << " ";
  }
  std::cout << std::endl;
}

void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  // glColor3f(0.0, 0.0, 0.0);

  // glBegin(GL_POLYGON);
  // glVertex3f(20.0, 20.0, 0.0);
  // glVertex3f(80.0, 20.0, 0.0);
  // glVertex3f(80.0, 80.0, 0.0);
  // glVertex3f(20.0, 80.0, 0.0);
  // glEnd();

  // todo: iterate over the global data structure, draw the polygons
  for (std::vector<unsigned> polygon : modelData.getPolygons()) {
    // static unsigned* test3 = polygon2;  //&polygon[0];
    static unsigned* test3 = &polygon[0];

    // glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, &polygon[0]);
    glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, test3);

    // unsigned* test3 = &polygon[0];
    for (int i = 0; i < 4; ++i) {
      std::cout << test3[i] << " ";
    }
    std::cout << std::endl;
  }
  // glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_INT, polygon2);

  glFlush();
}

void resize(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);
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
