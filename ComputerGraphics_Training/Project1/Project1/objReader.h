#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <stdio.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#pragma once
struct obj
{
	glm::vec3* vertex = NULL;
	int* face_v = NULL;
	int vertexNum = 0;
	int faceNum = 0;

	void ReadObj(FILE* objFile);
};