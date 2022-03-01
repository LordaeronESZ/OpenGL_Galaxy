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
GLuint sunTexture, earthTexture, moonTexture; // 纹理
Sphere mySphere;
Sphere earthSphere;
std::stack<glm::mat4> mvStack; // 矩阵堆栈

GLuint mvLoc, projLoc, nLoc; // 统一变量的引用
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat; // 变换矩阵

void setupVertices() {
	mySphere = Sphere(128);

	// 初始化 sun 的数据
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
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 顶点位置
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 纹理坐标
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); // 法向量
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]); // 索引
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);
}

void drawSphere(const GLuint& texture) {
	// 得到模型-视图矩阵和法向量变换矩阵
	mvMat = mvStack.top();
	invTrMat = glm::transpose(glm::inverse(mvMat));

	// 获取统一变量位置并赋值
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	// 指定顶点属性
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

	// 启用纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glEnable(GL_DEPTH_TEST); // 开启深度测试
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE); // 开启隐藏面消除
	glFrontFace(GL_CCW);

	glDrawElements(GL_TRIANGLES, mySphere.getNumIndices(), GL_UNSIGNED_INT, 0);
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 8.0f;
	sunLocX = 0.0f; sunLocY = 0.0f; sunLocZ = 0.0f;
	setupVertices();
	// 加载纹理贴图
	sunTexture = loadTexture("sun.jpg");
	earthTexture = loadTexture("earth.jpg");
	moonTexture = loadTexture("moon.jpg");
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(renderingProgram);

	// 构建投影矩阵
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees

	// 获取统一变量位置并赋值
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	
	// 构建模型-视图矩阵
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mvStack.push(vMat); // 将视图矩阵推入矩阵堆栈

	/// <summary>
	/// 绘制太阳
	/// </summary>
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(sunLocX, sunLocY, sunLocZ));
	mvStack.top() *= mMat;
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 1.0f, 0.0f)); // 太阳自转
	
	drawSphere(sunTexture);
	mvStack.pop(); // 矩阵堆栈中移除太阳自转
	
	/// <summary>
	/// 绘制地球
	/// </summary>
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime) * 4.0f, 0.0f, cos((float)currentTime) * 4.0f)); // 地球公转
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 1.0f, 0.0f)); // 地球自转
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)); // 地球缩放

	drawSphere(earthTexture);
	mvStack.pop(); // 矩阵堆栈中移除地球自转和缩放

	/// <summary>
	/// 绘制月球
	/// </summary>
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime) * 0.8f, cos((float)currentTime) * 0.8f)); // 月球公转
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 0.0f, 1.0f)); // 月球自转
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)); // 月球缩放

	drawSphere(moonTexture);
	mvStack.pop(); mvStack.pop(); mvStack.pop(); // 清空矩阵堆栈
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
	// glfwSwapInterval(1); // 开启 V-Sync
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