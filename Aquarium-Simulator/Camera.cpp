#include "Camera.h"

Camera::Camera(const int width, const int height, const glm::vec3& position)
{
	startPosition = position;
	Set(width, height, position);
}

void Camera::Set(const int width, const int height, const glm::vec3& position)
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
void Camera::Reset(const int width, const int height)
{
    Set(width, height, startPosition); 
}
void Camera::Reshape(int windowWidth, int windowHeight)
{
    width = windowWidth;
    height = windowHeight;

    // define the viewport transformation
    glViewport(0, 0, windowWidth, windowHeight);
}
const glm::vec3 Camera::GetPosition() const
{
    return position;
}
const glm::mat4 Camera::GetViewMatrix() const
{
    // Returns the View Matrix
    return glm::lookAt(position, position + forward, up);
}
const glm::mat4 Camera::GetProjectionMatrix() const
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
void Camera::ProcessKeyboard(ECameraMovementType direction, float deltaTime)
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
void Camera::MouseControl(float xPos, float yPos)
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
void Camera::ProcessMouseScroll(float yOffset)
{
    if (FoVy >= 1.0f && FoVy <= 90.0f) {
        FoVy -= yOffset;
    }
    if (FoVy <= 1.0f)
        FoVy = 1.0f;
    if (FoVy >= 90.0f)
        FoVy = 90.0f;
}
void Camera::renderAquarium()
{
    if (aquariumVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &aquariumVAO);
        glGenBuffers(1, &aquariumVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, aquariumVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(aquariumVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(aquariumVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void Camera::renderScene(const Shader& shader)
{
    // floor
    glm::mat4 model;
    shader.SetMat4("model", model);
    renderFloor();

    // cube
    model = glm::mat4();
    model = glm::translate(model, glm::vec3(0.0f, 1.75f, 0.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.SetMat4("model", model);
    renderCube();
    //2 cube
    model = glm::translate(model, glm::vec3(4.0f, 1.0f, 0.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.SetMat4("model", model);
    renderCube();
    //3 cube
    model = glm::translate(model, glm::vec3(-4.0f, 2.75f, 0.0));
    model = glm::rotate(model, glm::radians(157.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.SetMat4("model", model);
    renderCube();
    //4 cube
    model = glm::translate(model, glm::vec3(-2.0f, 0.75f, 1.0));
    model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.SetMat4("model", model);
    renderCube();
}
void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
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
void Camera::UpdateCameraVectors()
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
