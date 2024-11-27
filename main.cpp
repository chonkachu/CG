#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Object.h"
#include "Camera.h"

std::string loadShaderFromFile(const char* filePath) {
    std::string shaderCode;
    std::ifstream shaderFile;


    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {

        shaderFile.open(filePath);
        std::stringstream shaderStream;


        shaderStream << shaderFile.rdbuf();


        shaderFile.close();


        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    return shaderCode;
}

std::string vertexShader;
std::string fragmentShader;
bool polygonal_mode = false;
glm::vec3 eyeLeft(0.75865, 8.79846, 1.0f);
glm::vec3 eyeCentroid(0.03f, 8.79846, 1.0f);
glm::vec3 eyeDirection(0.0f);
glm::vec3 flashlightCentroid(-0.0432864, -0.05f, -0.274723);
glm::vec3 flashlightFront(-0.0432864, -0.05f, -0.574723);

class Renderer {
    public:
        static Renderer* instance;
        Renderer(int width, int height)
            : width(width), height(height), camera(nullptr)
        {
            instance = this;
            InitializeGLFW();
            InitializeOpenGL();
            InitializeShaders();
            LoadObjects();
            lightPos = glm::vec3(0.0f, 150.0f, 0.0f);
            dirLight.ambient = glm::vec3(0.2f);
            dirLight.diffuse = glm::vec3(0.02f);     
            dirLight.specular = glm::vec3(0.2f);   
            dirLight.direction = glm::normalize(glm::vec3(0.0f) - lightPos);
        }

        ~Renderer() {
            Cleanup();
        }

        void Run() {
            camera = new Camera(width, height, 70.0f, 4.0f, 0.0f);
            glfwSetCursorPos(window, width/2, height/2);

            while (!glfwWindowShouldClose(window)) {
                RenderFrame();
                camera->ProcessKeyboard(window);

                if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                    glfwSetWindowShouldClose(window, GL_TRUE);
            }
        }

        void HandleKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {

                if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
                    int index = key - GLFW_KEY_1;
                    if (index < objects.size()) {
                        selectedObjectIndex = index;
                        std::cout << "Selected object " << index << " " << objects[selectedObjectIndex]->name << std::endl;
                    }
                }

                if (key == GLFW_KEY_P) {
                    polygonal_mode = !polygonal_mode;
                }
                if (key == GLFW_KEY_O) {
                    ambientLightEnabled = !ambientLightEnabled;
                    dirLight.ambient = glm::vec3(0.1f);
                }
                if (key == GLFW_KEY_I && ambientLightEnabled) {
                    dirLight.ambient += glm::vec3(0.1f);
                    dirLight.ambient = glm::clamp(dirLight.ambient, glm::vec3(0.0f), glm::vec3(1.0f));
                }
                if (key == GLFW_KEY_U && ambientLightEnabled) {
                    dirLight.ambient -= glm::vec3(0.1f);
                    dirLight.ambient = glm::clamp(dirLight.ambient, glm::vec3(0.0f), glm::vec3(1.0f));
                }

                if (selectedObjectIndex >= 0 && selectedObjectIndex < objects.size()) {
                    Object* selectedObj = objects[selectedObjectIndex];
                    if (key == GLFW_KEY_Z) {
                        selectedObj->Rotate(rotationSpeed);
                    }
                    if (key == GLFW_KEY_C) {
                        selectedObj->Rotate(-rotationSpeed);
                    }
                    if (key == GLFW_KEY_RIGHT) {
                        selectedObj->Move(translationSpeed, 0.0f, 0.0f);
                    }
                    if (key == GLFW_KEY_LEFT) {
                        selectedObj->Move(-translationSpeed, 0.0f, 0.0f);
                    }
                    if (key == GLFW_KEY_UP) {
                        selectedObj->Move(0.0f, 0.0f, translationSpeed);
                    }
                    if (key == GLFW_KEY_DOWN) {
                        selectedObj->Move(0.0f, 0.0f, -translationSpeed);
                    }
                    if (key == GLFW_KEY_J) {
                        selectedObj->Move(0.0f, -translationSpeed, 0.0f);
                    }
                    if (key == GLFW_KEY_K) {
                        selectedObj->Move(0.0f, translationSpeed, 0.0f);
                    }
                    if (key == GLFW_KEY_V) {
                        selectedObj->Scale(0.1);
                    }
                    if (key == GLFW_KEY_B) {
                        selectedObj->Scale(-0.1);
                    }

                    if (key == GLFW_KEY_E) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.diffuseReflection += glm::vec3(0.1f);
                                mat.diffuseReflection = glm::clamp(mat.diffuseReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }

                    if (key == GLFW_KEY_R) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.diffuseReflection -= glm::vec3(0.1f);
                                mat.diffuseReflection = glm::clamp(mat.diffuseReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }


                    if (key == GLFW_KEY_E) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.diffuseReflection += glm::vec3(0.1f);
                                mat.diffuseReflection = glm::clamp(mat.diffuseReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }
                    if (key == GLFW_KEY_R) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.diffuseReflection -= glm::vec3(0.1f);
                                mat.diffuseReflection = glm::clamp(mat.diffuseReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }

                    if (key == GLFW_KEY_T) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.specularReflection += glm::vec3(0.1f);
                                mat.specularReflection = glm::clamp(mat.specularReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }
                    if (key == GLFW_KEY_Y) {  
                        for (auto &mat : selectedObj->materials) {
                            if (!mat.isLightSource) {
                                mat.specularReflection -= glm::vec3(0.1f);
                                mat.specularReflection = glm::clamp(mat.specularReflection, glm::vec3(0.0f), glm::vec3(1.0f));
                            }
                        }
                    }
                    if (key == GLFW_KEY_F) {
                        Object* selectedObj = objects[selectedObjectIndex];
                        bool hasLightSource = false;
                        for (const auto& mat : selectedObj->materials) {
                            if (mat.isLightSource) {
                                hasLightSource = true;
                                break;
                            }
                        }
                        if (hasLightSource) {
                            selectedObj->ToggleLights();
                        }
                    }
                }
            }
        }
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (instance) {
                instance->HandleKeyInput(window, key, scancode, action, mods);
            }
        }

    private:
        int width, height;
        GLFWwindow* window;
        GLuint shaderProgram;
        std::vector<Object*> objects;
        Camera* camera;
        int selectedObjectIndex = -1;  
        const float rotationSpeed = 0.05f;
        const float translationSpeed = 0.5f;

        glm::vec3 lightPos;  
        bool ambientLightEnabled = true;

        // Estruturas de Luz
        struct DirLight {
            glm::vec3 direction;
            glm::vec3 ambient;
            glm::vec3 diffuse;
            glm::vec3 specular;
        } dirLight;

        struct PointLight {
            glm::vec3 position;
            glm::vec3 color;
            float constant;
            float linear;
            float quadratic;
            glm::vec3 ambient;
            glm::vec3 diffuse;
            glm::vec3 specular;
        };

        struct SpotLight {
            glm::vec3 position;
            glm::vec3 direction;
            glm::vec3 color;

            float cutOff;
            float outerCutOff;

            float constant;
            float linear;
            float quadratic;

            glm::vec3 ambient;
            glm::vec3 diffuse;
            glm::vec3 specular;
        };

        std::vector<PointLight> pointLights;
        std::vector<SpotLight> spotLights;

        // Funções auxiliares OpenGL/GLFW
        void InitializeGLFW() {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

            window = glfwCreateWindow(width, height, "Iluminação", NULL, NULL);
            if (!window) {
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }

            glfwMakeContextCurrent(window);
            glfwSetKeyCallback(window, Renderer::KeyCallback);
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
            GLuint vs = CreateShader(GL_VERTEX_SHADER, vertexShader.c_str());
            GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fragmentShader.c_str());

            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vs);
            glAttachShader(shaderProgram, fs);
            glLinkProgram(shaderProgram);

            GLint success;
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
            }

            glDeleteShader(vs);
            glDeleteShader(fs);
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

        // Define objetos com texturas + propriedades difusas/especulares 
        void LoadObjects() {
            std::vector<const char*> skyTextures = {
                "textures/night.png",
            };
            std::vector<MaterialProperties> skyProperties = {
                MaterialProperties(
                        glm::vec3(0.0f),     
                        glm::vec3(0.9f),
                        glm::vec3(0.1f),
                        3.0f,                
                        false            
                        )
            };

            std::vector<const char*> treeTextures = {
                "textures/tree.jpg",
            };
            std::vector<MaterialProperties> treeProperties = {
                MaterialProperties(
                        glm::vec3(0.0f),     
                        glm::vec3(0.6f),     
                        glm::vec3(0.6f),     
                        1.0f,                
                        false                
                        )
            };


            std::vector<const char*> grassTextures = {
                "textures/grass.jpg",
            };
            std::vector<MaterialProperties> grassProperties = {
                MaterialProperties(
                        glm::vec3(0.0f),        
                        glm::vec3(1.0f),        
                        glm::vec3(1.0f),        
                        1.0f,                   
                        false                   
                        )
            };

            std::vector<const char*> houseTextures = {
                "textures/tramy-UVout.png",
                "textures/House-diff.png"
            };
            std::vector<MaterialProperties> houseProperties = {
                MaterialProperties(
                        glm::vec3(0.0f),        
                        glm::vec3(0.7f),        
                        glm::vec3(0.3f),        
                        32.0f,                  
                        false                   
                        ),
                MaterialProperties(
                        glm::vec3(0.0f),        
                        glm::vec3(0.6f),        
                        glm::vec3(0.2f),        
                        16.0f,                  
                        false                   
                        )
            };


            std::vector<const char*> lampTextures = {
                "textures/street_lamp_black.png",
                "textures/street_lamp_white.png",
                "textures/street_lamp_grey.png",
            };
            std::vector<MaterialProperties> lampProperties = {
                MaterialProperties(  
                        glm::vec3(0.0f),        
                        glm::vec3(0.2f),        
                        glm::vec3(0.8f),        
                        32.0f,                  
                        false                   
                        ),
                MaterialProperties(  
                        glm::vec3(1.0f, 0.9f, 0.7f) * 2.0f,  
                        glm::vec3(0.8f),        
                        glm::vec3(0.9f),        
                        1.0f,                   
                        true                    
                        ),
                MaterialProperties(  
                        glm::vec3(0.0f),        
                        glm::vec3(0.5f),        
                        glm::vec3(0.3f),        
                        16.0f,                  
                        false                   
                        )
            };

            std::vector<const char*> giantTextures = {
                "textures/laser_eyes.png",
                "textures/stone.png",
                "textures/stone.png",
                "textures/stone.png"
            };
            std::vector<MaterialProperties> giantProperties = {
                MaterialProperties(  
                        glm::vec3(0.0f, 1.0f, 0.0f) * 2.0f,  
                        glm::vec3(0.7f),        
                        glm::vec3(0.9f),        
                        1.0f,                   
                        true,                   
                        1.0f,                        
                        0.09f,                       
                        0.22f,                        
                        glm::cos(glm::radians(25.f)), 
                        glm::cos(glm::radians(30.f)), 
                        glm::vec3(0.0f, 1.0f, 1.0f)   
                        ),
                MaterialProperties(  
                        glm::vec3(0.0f),        
                        glm::vec3(0.6f),        
                        glm::vec3(0.2f),        
                        1.0f,                   
                        false                   
                        ),
                MaterialProperties(  
                        glm::vec3(0.0f),        
                        glm::vec3(0.6f),        
                        glm::vec3(0.2f),        
                        1.0f,                   
                        false                   
                        ),
                MaterialProperties(  
                        glm::vec3(0.0f),        
                        glm::vec3(0.6f),        
                        glm::vec3(0.2f),        
                        1.0f,                   
                        false                   
                        )

            };

            std::vector<const char*> flashlightTextures = {
                "textures/red.png",
                "textures/grey.jpg",
                "textures/red.png"
            };
            std::vector<MaterialProperties> flashlightProperties = {
                MaterialProperties(  
                        glm::vec3(0.0f),             
                        glm::vec3(0.7f),             
                        glm::vec3(0.3f),             
                        1.0f,                        
                        false                        
                        ),
                MaterialProperties(  
                        glm::vec3(0.8f, 0.8f, 0.8f),  
                        glm::vec3(0.8f),             
                        glm::vec3(0.9f),             
                        1.0f,                        
                        true,                        
                        1.0f,                        
                        0.09f,                       
                        0.001f,                        
                        glm::cos(glm::radians(25.f)), 
                        glm::cos(glm::radians(30.f)), 
                        glm::vec3(0.0f, 1.0f, 1.0f)   
                        ),
                MaterialProperties(  
                        glm::vec3(0.0f),             
                        glm::vec3(0.7f),             
                        glm::vec3(0.3f),             
                        1.0f,                        
                        false                        
                        )
            };

            std::vector<const char*> smallLampTextures = {
                "textures/brown.png",
                "textures/orange.jpg"
            };
            std::vector<MaterialProperties> smallLampProperties = {
                MaterialProperties(  
                        glm::vec3(0.0f),             
                        glm::vec3(0.6f),             
                        glm::vec3(0.4f),             
                        32.0f,                       
                        false                        
                        ),
                MaterialProperties(  
                        glm::vec3(1.0f, 0.5f, 0.0f),  
                        glm::vec3(0.8f),              
                        glm::vec3(0.9f),              
                        1.0f,                         
                        true,                         
                        0.0f,                         
                        0.1f,                         
                        0.22f                         
                        )
            };

            std::vector<const char*> thinkerTextures = {
                "textures/grey.jpg",
                "textures/grey.jpg"
            };

            std::vector<MaterialProperties> thinkerProperties = {
                MaterialProperties(
                        glm::vec3(0.02f),       
                        glm::vec3(0.5f),        
                        glm::vec3(0.7f),        
                        256.0f,                 
                        false
                        ),
                MaterialProperties(
                        glm::vec3(0.02f),       
                        glm::vec3(0.5f),        
                        glm::vec3(0.7f),        
                        256.0f,                 
                        false
                        )
            };

            std::vector<const char*> victoryTextures = {
                "textures/grey.jpg",
            };

            std::vector<MaterialProperties> victoryProperties = {
                MaterialProperties(
                        glm::vec3(0.02f),       
                        glm::vec3(0.3f),        
                        glm::vec3(0.8f),        
                        256.0f,                 
                        false
                        )
            };

            std::vector<const char*> nightstandTextures = {
                "textures/stand.png",
            };
            std::vector<MaterialProperties> nightstandProperties = {
                MaterialProperties(
                        glm::vec3(0.02f),       
                        glm::vec3(0.7f),        
                        glm::vec3(0.2f),        
                        32.0f,                  
                        false
                        )
            };

            std::vector<const char*> bedTextures = {
                "textures/bed.png",
                "textures/bed_sheet.png",
                "textures/pillows.png"
            };
            std::vector<MaterialProperties> bedProperties = {
                MaterialProperties(
                        glm::vec3(0.0f),       
                        glm::vec3(0.95f),        
                        glm::vec3(0.1f),        
                        2.0f,                   
                        false
                        ),
                MaterialProperties(
                        glm::vec3(0.0f),       
                        glm::vec3(0.95f),        
                        glm::vec3(0.1f),        
                        2.0f,                   
                        false
                        ),
                MaterialProperties(
                        glm::vec3(0.0f),       
                        glm::vec3(0.95f),        
                        glm::vec3(0.1f),        
                        2.0f,                   
                        false
                        )
            };
            objects.push_back(new Object(shaderProgram, "models/flashlight.obj", flashlightTextures,
                        flashlightProperties, -9.7f, 3.67f, 14.5f, 1.0f, 4.6f, 1));
            objects.push_back(new Object(shaderProgram, "models/small_lamp.obj", smallLampTextures,
                        smallLampProperties, 1.5f, 0.5f, -21.0f, 1.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/giant.obj", giantTextures,
                        giantProperties, 80.0f, 0.0f, -20.0f, 1.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                        lampProperties, 70.0f, 0.0f, -10.0f, 5.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                        lampProperties, 70.0f, 0.0f, 10.0f, 5.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/bed.obj", bedTextures,
                        bedProperties, -6.0f, 0.3f, -18.0f, 2.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/sphere.obj", skyTextures,
                        skyProperties, 0.0f, 0.0f, 0.0f, 200.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/tree.obj", treeTextures,
                        treeProperties, 63.0f, 0.0f, -18.0f, 0.3f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/grass.obj", grassTextures,
                        grassProperties, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/casa.obj", houseTextures,
                        houseProperties, 2.0f, 0.5f, -1.0f, 5.0f, 0.0f, 1));
            objects.push_back(new Object(shaderProgram, "models/thinker.obj", thinkerTextures,
                        thinkerProperties, 4.5f, 0.6f, -21.0f, 0.7f, 9.4f, 1));
            objects.push_back(new Object(shaderProgram, "models/victory.obj", victoryTextures,
                        victoryProperties, 13.0f, -0.75f, 14.0f, 0.5f, 3.7f, 1));
            objects.push_back(new Object(shaderProgram, "models/nightstand.obj", nightstandTextures,
                        nightstandProperties, -10.5f, 1.5f, 13.5f, 0.4f, 0.0f, 1));
        }

        void SetupLighting() {
            glm::vec3 targetPos(0.0f, 0.0f, 0.0f);  
            dirLight.direction = glm::normalize(targetPos - lightPos);
            dirLight.ambient = ambientLightEnabled ? dirLight.ambient : glm::vec3(0.0f);     

            GLint lightDirLoc = glGetUniformLocation(shaderProgram, "dirLight.direction");
            GLint lightAmbientLoc = glGetUniformLocation(shaderProgram, "dirLight.ambient");
            GLint lightDiffuseLoc = glGetUniformLocation(shaderProgram, "dirLight.diffuse");
            GLint lightSpecularLoc = glGetUniformLocation(shaderProgram, "dirLight.specular");

            glUniform3fv(lightDirLoc, 1, glm::value_ptr(dirLight.direction));
            glUniform3fv(lightAmbientLoc, 1, glm::value_ptr(dirLight.ambient));
            glUniform3fv(lightDiffuseLoc, 1, glm::value_ptr(dirLight.diffuse));
            glUniform3fv(lightSpecularLoc, 1, glm::value_ptr(dirLight.specular));

            pointLights.clear();
            spotLights.clear();

            // Pega os atributos dos materiais que são lightsources e salva no seu respectivo tipo
            for (const auto& obj : objects) {
                for (size_t i = 0; i < obj->materials.size(); i++) {
                    const auto& mat = obj->materials[i];
                    if (mat.isLightSource && mat.isActive) {
                        if (mat.cutOff > -0.9f) {  
                            SpotLight light;
                            glm::mat4 model = obj->GetModelMatrix();
                            // Define direção do raio de luz da lanterna e do gigante
                            if (obj->name == "models/giant.obj") {
                                glm::vec3 eyeCentroidNew = glm::vec3(model * glm::vec4(eyeCentroid, 1.0f));
                                glm::vec3 eyeLeftNew = glm::vec3(model * glm::vec4(eyeLeft, 1.0f));
                                light.position = eyeCentroidNew; 

                                glm::vec3 leftVector = eyeLeftNew-eyeCentroidNew;
                                glm::vec3 upVector(0.0f, 1.0f, 0.0f);
                                eyeDirection = glm::normalize(glm::cross(leftVector, upVector));
                                light.direction = eyeDirection;
                            }
                            else if (obj->name == "models/flashlight.obj") {
                                glm::vec3 flashlightCentroidNew = glm::vec3(model * glm::vec4(flashlightCentroid, 1.0f));
                                glm::vec3 flashlightFrontNew = glm::vec3(model * glm::vec4(flashlightFront, 1.0f));
                                light.position = flashlightCentroidNew;
                                light.direction = flashlightFrontNew - flashlightCentroidNew;
                            }
                            light.color = mat.emission;
                            light.cutOff = mat.cutOff;
                            light.outerCutOff = mat.outerCutOff;
                            light.constant = mat.constant;
                            light.linear = mat.linear;
                            light.quadratic = mat.quadratic;
                            light.ambient = glm::vec3(0.05f);
                            light.diffuse = glm::vec3(0.8f);
                            light.specular = glm::vec3(0.8f);
                            spotLights.push_back(light);
                        } 
                        else {
                            PointLight light;
                            glm::mat4 model = obj->GetModelMatrix();
                            light.position = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                            light.color = mat.emission;
                            light.constant = mat.constant;
                            light.linear = mat.linear;
                            light.quadratic = mat.quadratic;
                            light.ambient = glm::vec3(0.05f);
                            light.diffuse = glm::vec3(0.8f);
                            light.specular = glm::vec3(0.5f);
                            pointLights.push_back(light);
                        }
                    }
                }
            }

            glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), 
                    static_cast<GLint>(pointLights.size()));
            glUniform1i(glGetUniformLocation(shaderProgram, "numSpotLights"), 
                    static_cast<GLint>(spotLights.size()));

            // Carrega PointLights
            for (size_t i = 0; i < pointLights.size(); i++) {
                std::string base = "pointLights[" + std::to_string(i) + "].";

                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "position").c_str()),
                        1, glm::value_ptr(pointLights[i].position));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "color").c_str()),
                        1, glm::value_ptr(pointLights[i].color));

                glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()),
                        pointLights[i].constant);
                glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()),
                        pointLights[i].linear);
                glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()),
                        pointLights[i].quadratic);

                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "ambient").c_str()),
                        1, glm::value_ptr(pointLights[i].ambient));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "diffuse").c_str()),
                        1, glm::value_ptr(pointLights[i].diffuse));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "specular").c_str()),
                        1, glm::value_ptr(pointLights[i].specular));
            }

            // Carrega SpotLights
            for (size_t i = 0; i < spotLights.size(); i++) {
                std::string base = "spotLights[" + std::to_string(i) + "].";

                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "position").c_str()),
                        1, glm::value_ptr(spotLights[i].position));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "direction").c_str()),
                        1, glm::value_ptr(spotLights[i].direction));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "color").c_str()),
                        1, glm::value_ptr(spotLights[i].color));

                glUniform1f(glGetUniformLocation(shaderProgram, (base + "cutOff").c_str()),
                        spotLights[i].cutOff);
                glUniform1f(glGetUniformLocation(shaderProgram, (base + "outerCutOff").c_str()),
                        spotLights[i].outerCutOff);

                glUniform1f(glGetUniformLocation(shaderProgram, (base + "constant").c_str()),
                        spotLights[i].constant);
                glUniform1f(glGetUniformLocation(shaderProgram, (base + "linear").c_str()),
                        spotLights[i].linear);
                glUniform1f(glGetUniformLocation(shaderProgram, (base + "quadratic").c_str()),
                        spotLights[i].quadratic);

                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "ambient").c_str()),
                        1, glm::value_ptr(spotLights[i].ambient));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "diffuse").c_str()),
                        1, glm::value_ptr(spotLights[i].diffuse));
                glUniform3fv(glGetUniformLocation(shaderProgram, (base + "specular").c_str()),
                        1, glm::value_ptr(spotLights[i].specular));
            }
        }

        void RenderFrame() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

            glm::mat4 view = camera->GetViewMatrix();
            glm::mat4 projection = camera->GetProjectionMatrix();

            GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
            GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
            GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->GetPosition()));

            SetupLighting();

            glPolygonMode(GL_FRONT_AND_BACK, polygonal_mode ? GL_LINE : GL_FILL);

            for (auto obj : objects) {
                if (obj->name == "models/sphere.obj")
                    obj->Rotate(0.001f);

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

Renderer* Renderer::instance = nullptr;

int main() {
    vertexShader = loadShaderFromFile("vs.glsl");
    fragmentShader = loadShaderFromFile("fs.glsl");
    try {
        Renderer renderer(1920, 1080);
        renderer.Run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
