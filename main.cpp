#define GLEW_STATIC
#include<iostream>
#include<string>
#include<fstream>
#include<cmath>
#include<stack>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<SOIL2/SOIL2.h>
#include"Utils.h"
#include"Sphere.h"
constexpr int numVAOs = 1;
constexpr int numVBOs = 4;

float cameraX, cameraY, cameraZ;
float sunLocX, sunLocY, sunLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint sunTexture, earthTexture, moonTexture; // ����
Sphere mySphere;
Sphere earthSphere;
std::stack<glm::mat4> mvStack; // �����ջ

GLuint mvLoc, projLoc, nLoc; // ͳһ����������
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat; // �任����

void setupVertices() {
	mySphere = Sphere(128);

	// ��ʼ�� sun ������
	std::vector<int> ind = mySphere.getIndices();
	std::vector<glm::vec3> vert = mySphere.getVertices();
	std::vector<glm::vec2> tex = mySphere.getTexCoords();
	std::vector<glm::vec3> norm = mySphere.getNormals();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	for (int i = 0; i < mySphere.getNumVertices(); i++) {
		pvalues.emplace_back(vert[i].x);
		pvalues.emplace_back(vert[i].y);
		pvalues.emplace_back(vert[i].z);

		tvalues.emplace_back(tex[i].s);
		tvalues.emplace_back(tex[i].t);

		nvalues.emplace_back(norm[i].x);
		nvalues.emplace_back(norm[i].y);
		nvalues.emplace_back(norm[i].z);
	}

	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	
	glGenBuffers(numVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // ����λ��
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ��������
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); // ������
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]); // ����
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);
}

void drawSphere(const GLuint& texture) {
	// �õ�ģ��-��ͼ����ͷ������任����
	mvMat = mvStack.top();
	invTrMat = glm::transpose(glm::inverse(mvMat));

	// ��ȡͳһ����λ�ò���ֵ
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	// ָ����������
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);

	// ��������
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glEnable(GL_DEPTH_TEST); // ������Ȳ���
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE); // ��������������
	glFrontFace(GL_CCW);

	glDrawElements(GL_TRIANGLES, mySphere.getNumIndices(), GL_UNSIGNED_INT, 0);
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 8.0f;
	sunLocX = 0.0f; sunLocY = 0.0f; sunLocZ = 0.0f;
	setupVertices();
	// ����������ͼ
	sunTexture = loadTexture("sun.jpg");
	earthTexture = loadTexture("earth.jpg");
	moonTexture = loadTexture("moon.jpg");
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(renderingProgram);

	// ����ͶӰ����
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees

	// ��ȡͳһ����λ�ò���ֵ
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	
	// ����ģ��-��ͼ����
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mvStack.push(vMat); // ����ͼ������������ջ

	/// <summary>
	/// ����̫��
	/// </summary>
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(sunLocX, sunLocY, sunLocZ));
	mvStack.top() *= mMat;
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 1.0f, 0.0f)); // ̫����ת
	
	drawSphere(sunTexture);
	mvStack.pop(); // �����ջ���Ƴ�̫����ת
	
	/// <summary>
	/// ���Ƶ���
	/// </summary>
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime) * 4.0f, 0.0f, cos((float)currentTime) * 4.0f)); // ����ת
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 1.0f, 0.0f)); // ������ת
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)); // ��������

	drawSphere(earthTexture);
	mvStack.pop(); // �����ջ���Ƴ�������ת������

	/// <summary>
	/// ��������
	/// </summary>
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime) * 0.8f, cos((float)currentTime) * 0.8f)); // ����ת
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 0.0f, 1.0f)); // ������ת
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)); // ��������

	drawSphere(moonTexture);
	mvStack.pop(); mvStack.pop(); mvStack.pop(); // ��վ����ջ
}

int main() {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Galaxy with OpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		exit(EXIT_FAILURE);
	}
	// glfwSwapInterval(1); // ���� V-Sync
	init(window);
	double fpsTime = glfwGetTime();
	int framerate = 0;
	long long framerateSum = 0;
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (glfwGetTime() - fpsTime > 1) {
			system("cls");
			std::cout << "Cur FPS: " << framerate << std::endl;
			std::cout << "Avg FPS: " << framerateSum / fpsTime << std::endl;
			fpsTime = glfwGetTime();
			framerate = 0;
		}
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
		framerate++;
		framerateSum++;
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}