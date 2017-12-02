
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

#ifndef SHADER_H
#define SHADER_H

struct shader {
  GLuint vertShaderHandle, fragShaderHandle, programHandle;

  ~shader() {
    glDeleteShader(vertShaderHandle);
    glDeleteShader(fragShaderHandle);
    glDeleteProgram(programHandle);
  }

  void setShaders(char *vertShaderFileName, char *fragShaderFileName) {
    // create shader handles
    GLuint vertexShaderHandle(glCreateShader(GL_VERTEX_SHADER));
    GLuint fragmentShaderHandle(glCreateShader(GL_FRAGMENT_SHADER));

    // read shader programs into c strings
    char *vertexShaderSourceCodeString(readTxtFile(vertShaderFileName));
    char *fragmentShaderSourceCodeString(readTxtFile(fragShaderFileName));

    // set strings as shader sources
    glShaderSource(vertexShaderHandle, 1,
                   (const char **)&vertexShaderSourceCodeString, NULL);
    glShaderSource(fragmentShaderHandle, 1,
                   (const char **)&fragmentShaderSourceCodeString, NULL);

    // free shader source memory (no longer needed once loaded into GPU)
    if (vertexShaderHandle != 0)
      free(vertexShaderSourceCodeString);
    if (fragmentShaderHandle != 0)
      free(fragmentShaderSourceCodeString);

    // compile shader programs
    glCompileShader(vertexShaderHandle);
    glCompileShader(fragmentShaderHandle);

    // verify
    if (!glVerifyHandle(vertexShaderHandle))
      throw std::runtime_error("failed to compile the vertex shader");
    if (!glVerifyHandle(fragmentShaderHandle))
      throw std::runtime_error("failed to compile the fragment shader");

    // create program handle
    GLuint glProgramHandle(glCreateProgram());

    // attach shaders
    glAttachShader(glProgramHandle, vertexShaderHandle);
    glAttachShader(glProgramHandle, fragmentShaderHandle);

    // link program
    glLinkProgram(glProgramHandle);

    // verify
    if (!glVerifyHandle(glProgramHandle))
      throw std::runtime_error("failed to link shaders");

    // detach shaders (no longer needed once linked into program)
    glDetachShader(glProgramHandle, vertexShaderHandle);
    glDetachShader(glProgramHandle, fragmentShaderHandle);

    // install program into rendering state
    glUseProgram(glProgramHandle);

    glClearColor(1.0, 1.0, 1.0, 0.0);
  }

  char *readTxtFile(char *fileName) // reads a text file into a C string
  {
    ifstream infile(fileName, ios::binary);
    if (!infile)
      throw runtime_error(string("Cannot open file ") + fileName);
    infile.seekg(0, ios::end);
    size_t length(infile.tellg());
    char *data(new char[length + 1]);
    infile.seekg(0, ios::beg);
    infile.read(data, length);
    data[length] = '\0'; // end of string
    infile.close();
    return data;
  }

  int glVerifyHandle(GLuint handle) // verifies program attached to handle
  {
    int logLen, checkLen;
    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
    char *infoLog(new char[logLen]);
    glGetInfoLogARB(handle, logLen, &checkLen, &infoLog[0]);
    if (checkLen > 0) {
      cerr << "Error:" << endl << infoLog << endl;
      return 0; // failed
    }
    return 1; // okay
  }
};

#endif
