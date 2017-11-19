#include "texture.h"

void texture::glLoadTexture() {
  glGenTextures(1, textureIds);

  BitMapFile *skinBitMapFile;

  skinBitMapFile = getbmp(matp->getMapKdPath().getAsString());

  glBindTexture(GL_TEXTURE_2D, textureIds[0]);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, skinBitMapFile->sizeX,
               skinBitMapFile->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               skinBitMapFile->data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
