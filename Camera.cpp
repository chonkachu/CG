// Camera.cpp
#include "Camera.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <iostream>

Camera* Camera::instance = nullptr;

bool checkSkyBounds(glm::vec3 pos, glm::vec3 delta) {
    float abs = glm::length(pos+delta);

    return (abs < 198.0f);
}
bool checkFloorBounds(glm::vec3 pos, glm::vec3 delta) {
    float y = pos.y+delta.y;

    return (y > 1.0f);
}
Camera::Camera(float width, float height, float x, float y, float z) 
    : position(x, y, z)
    , front(0.0f, 0.0f, -1.0f)
    , up(0.0f, 1.0f, 0.0f)
    , yaw(-90.0f)
    , pitch(0.0f)
    , firstMouse(true)
    , lastX(width/2.0f)
    , lastY(height/2.0f)
    , width(width)
    , height(height)
{
    instance = this;
}

void Camera::ProcessMouseMovement(float xpos, float ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.3f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    UpdateCameraVectors();
}

void Camera::ProcessKeyboard(GLFWwindow* window) {
    float cameraSpeed = 0.5f;

    glm::vec3 delta;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        delta = cameraSpeed * front;
        if (checkSkyBounds(position, delta) && checkFloorBounds(position, delta))
            position += delta;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        delta = -cameraSpeed * front;
        if (checkSkyBounds(position, delta) && checkFloorBounds(position, delta))
            position += delta;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        delta = -glm::normalize(glm::cross(front, up)) * cameraSpeed;
        if (checkSkyBounds(position, delta) && checkFloorBounds(position, delta))
            position += delta;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        delta = glm::normalize(glm::cross(front, up)) * cameraSpeed;
        if (checkSkyBounds(position, delta) && checkFloorBounds(position, delta))
            position += delta;
    }
std::cout << position.x << " " << position.y << " " << position.z << " " << std::endl;
}

void Camera::UpdateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(45.0f), width / height, 0.1f, 1000.0f);
}
glm::vec3 Camera::GetPosition() const {
    return position;
}

void Camera::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (instance) {
        instance->ProcessMouseMovement(xpos, ypos);
    }
}
