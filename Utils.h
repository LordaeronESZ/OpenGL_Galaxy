#pragma once
#include<string>
#include<GLFW/glfw3.h>

GLuint createShaderProgram();

std::string readShaderSource(const char* filePath);

void printShaderLog(GLuint shader);

void printProgramLog(int prog);

bool checkOpenGLError();

GLuint loadTexture(const char* textImagePath);