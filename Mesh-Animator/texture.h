#pragma once

#include <string>

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "getbmp.h"
#include "material.h"

struct texture {
  unsigned int textureIds[1];

  material *matp;

  texture(material *matref) { matp = matref; }

  void glLoadTexture(void);
};
