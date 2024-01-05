// ViewOBJModel.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <stb_image.h>

#include <Windows.h>
#include <locale>
#include <codecvt>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include <map>

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Model.h"
#include "TransparentObj.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

enum ECameraMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
private:
	// Default camera values
	const float zNEAR = 0.1f;
	const float zFAR = 500.f;
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float FOV = 45.0f;
	glm::vec3 startPosition;

public:
	Camera(const int width, const int height, const glm::vec3& position)
	{
		startPosition = position;
		Set(width, height, position);
	}

	void Set(const int width, const int height, const glm::vec3& position)
	{
		this->isPerspective = true;
		this->yaw = YAW;
		this->pitch = PITCH;

		this->FoVy = FOV;
		this->width = width;
		this->height = height;
		this->zNear = zNEAR;
		this->zFar = zFAR;

		this->worldUp = glm::vec3(0, 1, 0);
		this->position = position;

		lastX = width / 2.0f;
		lastY = height / 2.0f;
		bFirstMouseMove = true;

		UpdateCameraVectors();
	}

	void Reset(const int width, const int height)
	{
		Set(width, height, startPosition);
	}

	void Reshape(int windowWidth, int windowHeight)
	{
		width = windowWidth;
		height = windowHeight;

		// define the viewport transformation
		glViewport(0, 0, windowWidth, windowHeight);
	}

	const glm::mat4 GetViewMatrix() const
	{
		// Returns the View Matrix
		return glm::lookAt(position, position + forward, up);
	}

	const glm::vec3 GetPosition() const
	{
		return position;
	}

	const glm::mat4 GetProjectionMatrix() const
	{
		glm::mat4 Proj = glm::mat4(1);
		if (isPerspective) {
			float aspectRatio = ((float)(width)) / height;
			Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
		}
		else {
			float scaleFactor = 2000.f;
			Proj = glm::ortho<float>(
				-width / scaleFactor, width / scaleFactor,
				-height / scaleFactor, height / scaleFactor, -zFar, zFar);
		}
		return Proj;
	}

	void ProcessKeyboard(ECameraMovementType direction, float deltaTime)
	{
		float velocity = (float)(cameraSpeedFactor * deltaTime);
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			break;
		}
	}

	void MouseControl(float xPos, float yPos)
	{
		if (bFirstMouseMove) {
			lastX = xPos;
			lastY = yPos;
			bFirstMouseMove = false;
		}

		float xChange = xPos - lastX;
		float yChange = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
			return;
		}
		xChange *= mouseSensitivity;
		yChange *= mouseSensitivity;

		ProcessMouseMovement(xChange, yChange);
	}

	void ProcessMouseScroll(float yOffset)
	{
		if (FoVy >= 1.0f && FoVy <= 90.0f) {
			FoVy -= yOffset;
		}
		if (FoVy <= 1.0f)
			FoVy = 1.0f;
		if (FoVy >= 90.0f)
			FoVy = 90.0f;
	}

private:
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
	{
		yaw += xOffset;
		pitch += yOffset;

		//std::cout << "yaw = " << yaw << std::endl;
		//std::cout << "pitch = " << pitch << std::endl;

		// Avem grijã sã nu ne dãm peste cap
		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Se modificã vectorii camerei pe baza unghiurilor Euler
		UpdateCameraVectors();
	}

	void UpdateCameraVectors()
	{
		// Calculate the new forward vector
		this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward.y = sin(glm::radians(pitch));
		this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward = glm::normalize(this->forward);
		// Also re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up = glm::normalize(glm::cross(right, forward));
	}

protected:
	const float cameraSpeedFactor = 5.0f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	// Euler Angles
	float yaw;
	float pitch;

	bool bFirstMouseMove = true;
	float lastX = 0.f, lastY = 0.f;
};

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {

	}
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


unsigned int transparentRectVAO = 0; 
unsigned int transparentRectVBO = 0;
void drawRect() {
	if (transparentRectVAO == 0) {
		float transparentVertices[] = {
			// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
			0.0f,  0.5f,  0.0f,
			0.0f, -0.5f,  0.0f,
			1.0f, -0.5f,  0.0f,

			0.0f,  0.5f,  0.0f,
			1.0f, -0.5f,  0.0f,
			1.0f,  0.5f,  0.0f
		};

		glGenVertexArrays(1, &transparentRectVAO);
		glGenBuffers(1, &transparentRectVBO);
		glBindVertexArray(transparentRectVAO);
		glBindBuffer(GL_ARRAY_BUFFER, transparentRectVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		/*glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));*/
		glBindVertexArray(0);
	}
	//draw aquarium
	glBindVertexArray(transparentRectVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int transparentSquareVAO = 0;
unsigned int transparentSquareRectVBO = 0;
void drawSquare() {
	if (transparentSquareVAO == 0) {
		float transparentVertices[] = {
			// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
			0.0f,  0.5f,  0.0f,
			0.0f, -0.5f,  0.0f,
			0.0f, -0.5f,  1.0f,

			0.0f,  0.5f,  0.0f,
			0.0f, -0.5f,  1.0f,
			0.0f,  0.5f,  1.0f
		};

		glGenVertexArrays(1, &transparentSquareVAO);
		glGenBuffers(1, &transparentSquareRectVBO);
		glBindVertexArray(transparentSquareVAO);
		glBindBuffer(GL_ARRAY_BUFFER, transparentSquareRectVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		/*glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));*/
		glBindVertexArray(0);
	}
	//draw aquarium
	glBindVertexArray(transparentSquareVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int transparentCeilingVAO = 0;
unsigned int transparentCeilingVBO = 0;
void drawCeiling() {
	if (transparentCeilingVAO == 0) {
		float transparentVertices[] = {
			// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
			0.0f, 0.0f,  0.0f,
			1.0f, 0.0f,  0.0f,
			1.0f, 0.0f, 1.0f,

			0.0f, 0.0f,  0.0f,
			1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f
		};
		glGenVertexArrays(1, &transparentCeilingVAO);
		glGenBuffers(1, &transparentCeilingVBO);
		glBindVertexArray(transparentCeilingVAO);
		glBindBuffer(GL_ARRAY_BUFFER, transparentCeilingVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		/*glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));*/
		glBindVertexArray(0);
	}
	//draw aquarium
	glBindVertexArray(transparentCeilingVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

std::string returnCurentPath() {
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);
	return currentPath;
}

unsigned int CreateTexture(const std::string& strTexturePath)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else {
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);

	return textureId;
}

std::string currentPath = returnCurentPath();

unsigned int planeVAO = 0;
void renderFloor()
{
	unsigned int planeVBO;

	if (planeVAO == 0) {
		// set up vertex data (and buffer(s)) and configure vertex attributes
		float planeVertices[] = {
			// positions            // normals         // texcoords
			0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  12.0f,  0.0f,
			20.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
			20.0f, 0.0f, 6.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

			0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  12.0f,  0.0f,
			20.0f, 0.0f, 6.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
			0.0f, 0.0f, 6.0f,  0.0f, 1.0f, 0.0f,  12.0f, 5.0f
		};
		// plane VAO
		unsigned int planeVBO;
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindVertexArray(0);
	}

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}



glm::vec3 fishPos(0.0f, 0.0f, 0.0f);
float fishIncrement = -0.005f;
float fishRotation = 0.0f;
float fishRotationIncrement = 1.0f;

//declare model
Model fishObjModel;
Model coralObjModel;
Model starFishModel;
Model starFishModelGlass;
Model fish2ObjModel;
Model coral2ObjModel;
Model diverObjModel;
Model skullObjModel;
Model castleObjModel;
Model turtleObjModel;
Model treasureObjModel;

float incrementMoveSpeed = 0.01;
float incrementRotationSpeed = 0.4;

void renderScene(Shader& shader){
	//test draw floor
	{
		glm::mat4 model{ glm::mat4(1.0f) };
		model = glm::translate(model, glm::vec3(0.0f));
		shader.setMat4("model", model);
		renderFloor();
	}
	glm::mat4 model{ glm::mat4(1.0f) };
	fishObjModel.moveObject(incrementMoveSpeed, incrementRotationSpeed);
	model = glm::translate(glm::mat4(1.0f), fishObjModel.currentPos);
	model = glm::scale(model, glm::vec3(0.03f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fishObjModel.rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	shader.setMat4("model", model);
	fishObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, 0.0f, 3.3f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.03f));
	shader.setMat4("model", model);
	coralObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 4.3f));
	shader.setMat4("model", model);
	starFishModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(19.88f, 2.0f, 4.3f));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	starFishModelGlass.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(16.0f, 2.5f, 3.3f));
	model = glm::scale(model, glm::vec3(0.1f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fishRotation), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.setMat4("model", model);
	fish2ObjModel.Draw(shader); 
	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.0f, 0.65f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.06f));
	shader.setMat4("model", model);
	coral2ObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(14.0f, 0.0f, 5.5f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.11f));
	shader.setMat4("model", model);
	diverObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(19.4f, 0.0f, 0.5f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f));
	shader.setMat4("model", model);
	skullObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(13.0f, 0.0f, 2.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.16f));
	shader.setMat4("model", model);
	castleObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 2.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f));
	shader.setMat4("model", model);
	turtleObjModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.7f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.05f));
	shader.setMat4("model", model);
	treasureObjModel.Draw(shader);
}



int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Aquarium", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	//skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(10.0f, 3.0f, 10.0f));

	Shader shadowMappingShader((currentPath + "\\Shaders\\ShadowMapping.vs").c_str(), (currentPath + "\\Shaders\\ShadowMapping.fs").c_str());
	Shader shadowMappingDepthShader((currentPath + "\\Shaders\\ShadowMappingDepth.vs").c_str(), (currentPath + "\\Shaders\\ShadowMappingDepth.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());
	Shader windowShader((currentPath + "\\Shaders\\Blending.vs").c_str(), (currentPath + "\\Shaders\\Blending.fs").c_str());
	Shader skyboxShader((currentPath + "\\Shaders\\Skybox.vs").c_str(), (currentPath + "\\Shaders\\Skybox.fs").c_str());
	Shader debugQuad((currentPath + "\\Shaders\\DebugQuad.vs").c_str(), (currentPath + "\\Shaders\\DebugQuad.fs").c_str());

	//load aquarium texture
	//unsigned int windowTexture = CreateTexture((currentPath + "\\Textures\\ps-neutral.png").c_str());
	unsigned int floorTexture = CreateTexture((currentPath + "\\Textures\\gravel.png").c_str());

	//*********************************
	//model loading
	// --------------------
	std::string fishObjFileName = (currentPath + "\\Models\\Fish\\12265_Fish_v1_L2.obj");
	fishObjModel = Model(fishObjFileName, false);
	fishObjModel.setPos(glm::vec3(6.0f, 1.0f, 3.3f), glm::vec3(10.0f, 4.0f, 1.3f), 180.0f);

	std::string fish2ObjFileName = (currentPath + "\\Models\\fish2\\13007_Blue-Green_Reef_Chromis_v2_l3.obj");
	fish2ObjModel = Model(fish2ObjFileName, false);

	std::string coralObjFileName = (currentPath + "\\Models\\Coral\\10010_Coral_v1_L3.obj");
	coralObjModel = Model(coralObjFileName, false);

	std::string starFishFileName = (currentPath + "\\Models\\StarFish\\MPS0HO5HRW7HPKO72X12940HW.obj");
	starFishModel = Model(starFishFileName, false);

	std::string starFishGlassFileName = (currentPath + "\\Models\\StarFishGlass\\O5T6WV158SFXN8YLU5HYHWW1Q.obj");
	starFishModelGlass = Model(starFishGlassFileName, false);

	std::string coral2ObjFileName = (currentPath + "\\Models\\coral1\\20941_Brain_Coral_v1_NEW1.obj");
	coral2ObjModel = Model(coral2ObjFileName, false);

	std::string diverObjFileName = (currentPath + "\\Models\\diver\\13018_Aquarium_Deep_Sea_Diver_v1_L1.obj");
	diverObjModel = Model(diverObjFileName, false);

	std::string skullObjFileName = (currentPath + "\\Models\\skull\\13022_Aquarium_Skull_v1_L1.obj");
	skullObjModel = Model(skullObjFileName, false);

	std::string castleObjFileName = (currentPath + "\\Models\\Castle\\13020_Aquarium_Castle_v1_L1.obj");
	castleObjModel = Model(castleObjFileName, false);

	std::string turtleObjFileName = (currentPath + "\\Models\\Turtle\\20446_Sea_Turtle_v1 Textured.obj");
    turtleObjModel= Model(turtleObjFileName, false);

	std::string treasureObjFileName = (currentPath + "\\Models\\treasure\\13019_aquarium_treasure_chest_v1_L2.obj");
	treasureObjModel = Model(treasureObjFileName, false);


	//load skybox
	vector<std::string> faces
	{
		currentPath +  "\\Textures\\Skybox\\right.jpg",
		currentPath + "\\Textures\\Skybox\\left.jpg",
		currentPath + "\\Textures\\Skybox\\bottom.jpg",
		currentPath + "\\Textures\\Skybox\\top.jpg",
		currentPath + "\\Textures\\Skybox\\front.jpg",
		currentPath + "\\Textures\\Skybox\\back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// shader configuration
    // --------------------
	windowShader.use();
	windowShader.setInt("texture1", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	shadowMappingShader.use();
	shadowMappingShader.setInt("diffuseTexture", 0);
	shadowMappingShader.setInt("shadowMap", 1);

	

	// lighting info
   // -------------
	glm::vec3 lightPos(14.0f, 5.0f, 2.5f);

	glEnable(GL_CULL_FACE);

	

	//transparent objects location
	vector<TransparentObj> transparentObjects;
	const float aquariumLength = 20.0f;
	const float aquariumWidth = 6.0f;
	const float aquariumHeight = 5.0f;

	for (float i = 0.0f; i < aquariumLength; i += 1.0f) {
		for (float j = 0.0f; j < aquariumHeight; j += 1.0f) {
			transparentObjects.push_back(TransparentObj(glm::vec3(i, j, -0.0001f), TransparentObj::Type::GLASS, TransparentObj::WindowType::RECT));
			transparentObjects.push_back(TransparentObj(glm::vec3(i, j, aquariumWidth), TransparentObj::Type::GLASS, TransparentObj::WindowType::RECT));
		}
	}
	for (float i = 0.0f; i < aquariumWidth; i += 1.0f) {
		for (float j = 0.0f; j < aquariumHeight; j += 1.0f) {
			transparentObjects.push_back(TransparentObj(glm::vec3(-0.0001f, j, i), TransparentObj::Type::GLASS, TransparentObj::WindowType::SQUARE));
			transparentObjects.push_back(TransparentObj(glm::vec3(aquariumLength, j, i), TransparentObj::Type::GLASS, TransparentObj::WindowType::SQUARE));
		}
	}
	for (float i = 0.0f; i < aquariumLength; i += 1.0f) {
		for (float j = 0.0f; j < aquariumWidth; j += 1.0f) {
			transparentObjects.push_back(TransparentObj(glm::vec3(i, aquariumHeight - 1.5f, j), TransparentObj::Type::GLASS, TransparentObj::WindowType::CEILING));
		}
	}
	

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// change light position over time
		//lightPos.x = sin(glfwGetTime()) * 0.00000000000000001f;
		//lightPos.z = cos(glfwGetTime()) * 0.00000000000000001f;
		//lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

		//sort transparent objects
		std::multimap<float, TransparentObj*> sortedMap;
		for (int i = 0; i < transparentObjects.size(); i++) {
 			float distance = glm::length(pCamera->GetPosition() - transparentObjects[i].pos);
			sortedMap.insert(std::make_pair(distance, &(transparentObjects[i])));
		}

		
		//render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 20.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// render scene from light's point of view
		shadowMappingDepthShader.use();
		shadowMappingDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		renderScene(shadowMappingDepthShader);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render the scene
		// 2. render scene as normal using the generated depth/shadow map 
		
		shadowMappingShader.use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix();
		shadowMappingShader.setMat4("projection", projection);
		shadowMappingShader.setMat4("view", view);
		// set light uniforms
		shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
		shadowMappingShader.SetVec3("lightPos", lightPos);
		shadowMappingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDisable(GL_CULL_FACE);
		renderScene(shadowMappingShader);

		// render Depth map to quad for visual debugging
		// ---------------------------------------------
		/*debugQuad.use();
		debugQuad.setFloat("near_plane", near_plane);
		debugQuad.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderQuad();*/

		{
			// draw skybox
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			skyboxShader.use();
			glm::mat4 view = glm::mat4(glm::mat3(pCamera->GetViewMatrix())); // remove translation from the view matrix
			view = glm::scale(view, glm::vec3(-1.0f, -1.0f, -1.0f));
			skyboxShader.setMat4("view", view);
			skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());
			// skybox cube
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}
		
		//draw transparent obj
		//glBindTexture(GL_TEXTURE_2D, windowTexture);
		windowShader.use();
		projection = pCamera->GetProjectionMatrix();
		view = pCamera->GetViewMatrix();
		windowShader.setMat4("projection", projection);
		windowShader.setMat4("view", view);

		//furthest object is drawn first
		for (std::multimap<float, TransparentObj*>::reverse_iterator it = sortedMap.rbegin(); it != sortedMap.rend(); ++it)
		{
			if (it->second->type == TransparentObj::Type::GLASS) {
				glm::mat4 model = glm::mat4(1.0f);
				glm::vec4 color(0.133f, 0.424f, 1.0f, 0.2f); //default aquarium color
				model = glm::translate(model, it->second->pos);
				//bootom layer = brown
				if (it->second->pos.y == 0)
					color = glm::vec4(0.078f, 0.031f, 0.008f, 1.0f);
				//top layer light gray
				if (it->second->pos.y == aquariumHeight - 1.0f)
					color = glm::vec4(0.773, 0.843, 0.929, 0.2f);

				windowShader.setMat4("model", model);
				windowShader.SetVec4("color", color);
				switch (it->second->windowType) {
					case TransparentObj::WindowType::SQUARE:
						drawSquare();
						break;
					case TransparentObj::WindowType::RECT:
						drawRect();
						break;
					case TransparentObj::WindowType::CEILING:
						drawCeiling();
						break;
				}
			}
		}
		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}
