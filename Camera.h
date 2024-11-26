// Camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera(float width, float height, float x, float y, float z);
    
    void ProcessMouseMovement(float xpos, float ypos);
    void ProcessKeyboard(GLFWwindow* window);
    
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static Camera* GetInstance() { return instance; }

private:
    static Camera* instance;
    
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    
    float yaw;
    float pitch;
    bool firstMouse;
    float lastX, lastY;
    float width, height;
    
    void UpdateCameraVectors();
};
#endif
