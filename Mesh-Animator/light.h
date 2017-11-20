#pragma once

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

struct light {
  light() {}

  void glConfigureLighting() {
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
  }
};
