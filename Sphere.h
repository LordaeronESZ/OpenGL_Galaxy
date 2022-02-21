#pragma once
#include<iostream>
#include<cmath>
#include<vector>
#include<glm/glm.hpp>

class Sphere {
private:
	int numVertices;
	int numIndices;
	int radius;
	std::vector<int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	void init(int prec);
public:
	Sphere();
	Sphere(int prec);
	static float toRadians(float degrees);
	int getNumVertices();
	int getNumIndices();
	std::vector<int> getIndices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
};
