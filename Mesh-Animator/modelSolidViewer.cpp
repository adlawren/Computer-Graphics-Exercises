// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

// modelSolidViewer <mesh.obj>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "camera.h"
#include "mesh.h"

// todo: use
#include "light.h"
#include "texture.h"

using namespace Eigen;

int main(int, char **);
void setup(char *);
void drawScene(void);
void resize(int, int);
void keyInput(unsigned char, int, int);
void specialKeyInput(int, int, int);

/* global variables */
mesh obj;
camera cam;
Vector3f initialPosition(0.0, 0.0, -1.0);
bool fog = true;
const float fogColor[4] = {0.0, 0.0, 0.0, 0.0};

// todo: move?
enum DISPLAY_MODE { WIRE_FRAME, SHADED_FLAT, SHADED_SMOOTH, TEXTURED };

DISPLAY_MODE displayMode = DISPLAY_MODE::WIRE_FRAME;
void toggleDisplayMode() {
  if (displayMode == DISPLAY_MODE::TEXTURED) {
    displayMode = DISPLAY_MODE::WIRE_FRAME;
  } else {
    displayMode = static_cast<DISPLAY_MODE>(((int)displayMode) + 1);
  }
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitContextVersion(3, 0);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("modelViewer");
  glutDisplayFunc(drawScene);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  glutSpecialFunc(specialKeyInput);
#ifndef __APPLE__
  glewExperimental = GL_TRUE;
  glewInit();
#endif
  if (argc != 2) {
    cerr << "Usage: modelViewer <meshfile.obj>" << endl;
    exit(1);
  }
  setup(argv[1]);
  glutMainLoop();
  return 0;
}

void setup(char *fileName) {
  obj.readObjFile(fileName);
  obj.normalize();

  obj.glCreateDisplayList();

  cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);

  //// Configure lighting
  glEnable(GL_LIGHTING);

  float lightAmb[] = {0.0, 0.0, 0.0, 1.0};
  float lightDifAndSpec[] = {1.0, 1.0, 1.0, 1.0};
  float lightPos[] = {-5.0, 15.0, 3.0, 1.0};
  float globAmb[] = {0.2, 0.2, 0.2, 1.0};

  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  glEnable(GL_LIGHT0);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  float matAmbAndDif[] = {0.0, 0.0, 0.0, 1.0};
  float matSpec[] = {1.0, 1.0, 1.0, 1.0};
  float matShine[] = {50.0};

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
}

void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  cam.glVolume(); // camera view volume

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cam.glPosition();                // camera transformation
  obj.glPosition(initialPosition); // object transformation

  if (fog) { // set fog
    glEnable(GL_FOG);
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogi(GL_FOG_START, 1.0);
    glFogi(GL_FOG_END, 5.0);
  } else
    glDisable(GL_FOG);

  // draw model
  switch (displayMode) {
  case DISPLAY_MODE::WIRE_FRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case DISPLAY_MODE::SHADED_FLAT:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_FLAT);
    break;
  case DISPLAY_MODE::SHADED_SMOOTH:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
    break;
  case DISPLAY_MODE::TEXTURED:
    // todo
    break;
  default:
    break;
  }

  obj.glCallDisplayList();

  glutSwapBuffers();
}

void resize(int w, int h) { glViewport(0, 0, (GLsizei)w, (GLsizei)h); }

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
  case 'q':
    exit(0);
    break; // quit
  case 'w':
    obj.writeObjFile("output.obj");
    break;
  case 'n':
    obj.zTransl(-0.1);
    break;
  case 'N':
    obj.zTransl(0.1);
    break;
  case 'p':
    obj.xRotate(-10.0);
    break;
  case 'P':
    obj.xRotate(10.0);
    break;
  case 'y':
    obj.yRotate(-10.0);
    break;
  case 'Y':
    obj.yRotate(10.0);
    break;
  case 'r':
    obj.zRotate(-10.0);
    break;
  case 'R':
    obj.zRotate(10.0);
    break;
  case 'f':
    fog = false;
    break; // toggle fog off
  case 'F':
    fog = true;
    break;  // toggle fog on
  case 'x': // reset
    obj.reset();
    cam.initialize(persp, -0.1, 0.1, -0.1, 0.1, 0.1, 100.0);
    fog = false;
    break;
  case 's':
    toggleDisplayMode();
    break;
  default:
    break;
  }
  cam.keyInput(key); // camera controls

  glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_LEFT:
    obj.xTransl(-0.1);
    break;
  case GLUT_KEY_RIGHT:
    obj.xTransl(0.1);
    break;
  case GLUT_KEY_DOWN:
    obj.yTransl(-0.1);
    break;
  case GLUT_KEY_UP:
    obj.yTransl(0.1);
    break;
  default:
    break;
  }
  glutPostRedisplay();
}
