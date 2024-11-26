#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Object.h"
#include "Camera.h"

const char* vertexShaderSource = R"(
    attribute vec3 position;
    attribute vec2 texture_coord;
    varying vec2 out_texture;
            
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;        
    
    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);
        out_texture = vec2(texture_coord);
    }
)";

const char* fragmentShaderSource = R"(
    uniform vec4 color;
    varying vec2 out_texture;
    uniform sampler2D samplerTexture;
    
    void main() {
        vec4 texture = texture2D(samplerTexture, out_texture);
        gl_FragColor = texture;
    }
)";

bool polygonal_mode = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        polygonal_mode = !polygonal_mode;
    }
}

class Renderer {
public:
    Renderer(int width, int height)
        : width(width), height(height), camera(nullptr)
    {
        InitializeGLFW();
        InitializeOpenGL();
        InitializeShaders();
        LoadObjects();
    }

    ~Renderer() {
        Cleanup();
    }

    void Run() {
        camera = new Camera(width, height, 120.771, 10.7901, 6.4414);
        glfwSetCursorPos(window, width/2, height/2);

        while (!glfwWindowShouldClose(window)) {
            RenderFrame();
            camera->ProcessKeyboard(window);

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

private:
    int width, height;
    GLFWwindow* window;
    GLuint shaderProgram;
    std::vector<Object*> objects;
    Camera* camera;

    void InitializeGLFW() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        window = glfwCreateWindow(width, height, "Malhas e Texturas", NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, Camera::MouseCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void InitializeOpenGL() {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
    }

    void InitializeShaders() {
        GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glUseProgram(shaderProgram);
    }

    GLuint CreateShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
        }

        return shader;
    }

    void LoadObjects() {

        std::vector<const char*> houseTextures = {
            "textures/tramy-UVout.png",
            "textures/House-diff.png"
        };
        objects.push_back(new Object(shaderProgram, "models/casa.obj", houseTextures,
                                   2.0f, 0.5f, -1.0f, 7.0f, 1));

        std::vector<const char*> skyTextures = {
            "textures/night.png",
        };
        objects.push_back(new Object(shaderProgram, "models/sphere.obj", skyTextures,
                                   0.0f, 0.0f, 0.0f, 200.0f, 1));

        std::vector<const char*> grassTextures = {
            "textures/grass.jpg",
        };
        objects.push_back(new Object(shaderProgram, "models/grass.obj", grassTextures,
                                   0.0f, 0.0f, 0.0f, 2.0f, 1));

        std::vector<const char*> lampTextures = {
            "textures/street_lamp_black.png",
            "textures/street_lamp_white.png",
            "textures/street_lamp_grey.png",
        };
        objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                                   70.0f, 0.0f, -10.0f, 5.0f, 1));

        objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                                   70.0f, 0.0f, 10.0f, 5.0f, 1));
    }

    void RenderFrame() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();

        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glPolygonMode(GL_FRONT_AND_BACK, polygonal_mode ? GL_LINE : GL_FILL);

        for (auto obj : objects) {
            if (obj->name == "models/sphere.obj")
                obj->Rotate(0.003f);
            obj->Draw(polygonal_mode);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    void Cleanup() {
        for (auto obj : objects) {
            delete obj;
        }
        objects.clear();

        glDeleteProgram(shaderProgram);
        delete camera;
        glfwTerminate();
    }
};

int main() {
    try {
        Renderer renderer(1600, 1200);
        renderer.Run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
