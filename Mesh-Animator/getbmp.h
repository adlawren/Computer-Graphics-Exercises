// Written by Sumanta Guha, in the textbook "Computer Graphics Through OpenGL"

#ifndef GETBMP_H
#define GETBMP_H

using namespace std;

struct BitMapFile
{
   int sizeX;
   int sizeY;
   unsigned char *data;
};

BitMapFile *getbmp(string filename);

#endif
