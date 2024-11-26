#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Object.h"
#include "Camera.h"

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec2 texture_coord;
    layout (location = 2) in vec3 normal;

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        FragPos = vec3(model * vec4(position, 1.0));
        Normal = mat3(transpose(inverse(model))) * normal;
        TexCoords = texture_coord;
        
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    struct Material {
        sampler2D diffuse;
        vec3 emission;    // emission color/intensity
        float shininess;
        bool isLightSource;
        
        // Point light properties
        float constant;
        float linear;
        float quadratic;
    }; 

    struct DirLight {
        vec3 direction;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    struct PointLight {
        vec3 position;
        vec3 color;     // Light color
        
        float constant;
        float linear;
        float quadratic;
    
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };

    #define MAX_LIGHTS 10
    uniform int numLights;
    uniform PointLight pointLights[MAX_LIGHTS];
    uniform DirLight dirLight;

    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    uniform vec3 viewPos;
    uniform Material material;

    // Function prototypes
    vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
    vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

    void main()
    {
        // Handle light-emitting materials
        if (material.isLightSource) {
            vec3 texColor = vec3(texture(material.diffuse, TexCoords));
            FragColor = vec4(texColor * material.emission, 1.0);
            return;
        }

        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);

        // Phase 1: Directional light
        vec3 result = CalcDirLight(dirLight, norm, viewDir);

        // Phase 2: Point lights
        for(int i = 0; i < numLights; i++) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        }
        
        FragColor = vec4(result, 1.0);
    }

    // Calculates directional light contribution
    vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
    {
        vec3 lightDir = normalize(-light.direction);
        
        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        
        // Combine results
        vec3 texColor = vec3(texture(material.diffuse, TexCoords));
        vec3 ambient = light.ambient * texColor;
        vec3 diffuse = light.diffuse * diff * texColor;
        vec3 specular = light.specular * spec * texColor;
        
        return (ambient + diffuse + specular);
    }

    // Calculates point light contribution
    vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
    {
        vec3 lightDir = normalize(light.position - fragPos);
        
        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        
        // Specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        
        // Attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                                 light.quadratic * (distance * distance));
        
        // Combine results
        vec3 texColor = vec3(texture(material.diffuse, TexCoords));
        vec3 ambient = light.ambient * texColor;
        vec3 diffuse = light.diffuse * diff * texColor;
        vec3 specular = light.specular * spec * texColor;
        
        // Apply attenuation and light color
        vec3 colored = (ambient + diffuse + specular) * light.color;
        return colored * attenuation;
    }

)";

bool polygonal_mode = false;
bool showDebugLines = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        polygonal_mode = !polygonal_mode;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        showDebugLines = !showDebugLines;
    }
}

class LightDirectionVisualizer {
public:
    LightDirectionVisualizer(GLuint shaderProgram) {
        // Create a simple line geometry
        float vertices[] = {
            0.0f, 0.0f, 0.0f,    // Start point
            0.0f, -10.0f, 0.0f   // End point (scaled direction)
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }


    void Draw(const glm::mat4& view, const glm::mat4& projection, 
              const glm::vec3& lightPos, const glm::vec3& direction,
              GLuint shaderProgram) {
        // Create model matrix to position the line
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        
        // Send matrices to shader
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the line
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, 2);
    }

    ~LightDirectionVisualizer() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

private:
    GLuint vao, vbo;
};

class Renderer {
public:
    Renderer(int width, int height)
        : width(width), height(height), camera(nullptr), lightVisualizer(nullptr)
    {
        InitializeGLFW();
        InitializeOpenGL();
        InitializeShaders();
        LoadObjects();
        lightVisualizer = new LightDirectionVisualizer(shaderProgram);
        lightPos = glm::vec3(0.0f, 150.0f, 0.0f);
        dirLight.direction = glm::normalize(glm::vec3(0.0f) - lightPos);
    }

    ~Renderer() {
        delete lightVisualizer;
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

    LightDirectionVisualizer* lightVisualizer;
    glm::vec3 lightPos;  // Store light position

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

    std::vector<PointLight> pointLights;

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
        // House with regular materials (non-emissive)
        std::vector<const char*> houseTextures = {
            "textures/tramy-UVout.png",
            "textures/House-diff.png"
        };
        std::vector<MaterialProperties> houseProperties = {
            MaterialProperties(
                    glm::vec3(0.0f),     // No emission
                    32.0f,               // Moderate shininess
                    false                // Not a light source
                    ),
            MaterialProperties(
                    glm::vec3(0.0f),     // No emission
                    16.0f,               // Less shiny
                    false                // Not a light source
                    )
        };
        objects.push_back(new Object(shaderProgram, "models/casa.obj", houseTextures,
                    houseProperties, 2.0f, 0.5f, -1.0f, 7.0f, 1));

        // Night sky with subtle glow
        std::vector<const char*> skyTextures = {
            "textures/night.png",
        };
        std::vector<MaterialProperties> skyProperties = {
            MaterialProperties(
                    glm::vec3(0.1f),     // Very subtle emission
                    1.0f,                // Not shiny
                    true,                // Is a light source
                    1.0f,                // No distance falloff for sky
                    0.0f, 
                    0.0f
                    )
        };
        objects.push_back(new Object(shaderProgram, "models/sphere.obj", skyTextures,
                    skyProperties, 0.0f, 0.0f, 0.0f, 200.0f, 1));

        // Grass (non-emissive, matte)
        std::vector<const char*> grassTextures = {
            "textures/grass.jpg",
        };
        std::vector<MaterialProperties> grassProperties = {
            MaterialProperties(
                    glm::vec3(0.0f),     // No emission
                    2.0f,                // Very low shininess for matte look
                    false                // Not a light source
                    )
        };
        objects.push_back(new Object(shaderProgram, "models/grass.obj", grassTextures,
                    grassProperties, 0.0f, 0.0f, 0.0f, 2.0f, 1));

        // Street lamps with emissive light bulbs
        std::vector<const char*> lampTextures = {
            "textures/street_lamp_black.png",
            "textures/street_lamp_white.png",
            "textures/street_lamp_grey.png",
        };
        std::vector<MaterialProperties> lampProperties = {
            MaterialProperties(  // Black metal parts
                    glm::vec3(0.0f),     // No emission
                    32.0f,               // Metallic shininess
                    false                // Not a light source
                    ),
            MaterialProperties(  // Light bulb
                    glm::vec3(1.0f, 0.9f, 0.7f) * 2.0f,  // Warm bright light
                    1.0f,                // Not shiny
                    true,                // Is a light source
                    0.0f,                // Attenuation factors for ~20 unit radius
                    0.1f,               // Medium distance falloff
                    0.005f                // Quadratic falloff
                    ),
            MaterialProperties(  // Grey parts
                    glm::vec3(0.0f),     // No emission
                    16.0f,               // Less shiny
                    false                // Not a light source
                    )
        };

        // Create the lamps
        objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                    lampProperties, 70.0f, 0.0f, -10.0f, 5.0f, 1));

        objects.push_back(new Object(shaderProgram, "models/lamp.obj", lampTextures,
                    lampProperties, 70.0f, 0.0f, 10.0f, 5.0f, 1));

        std::vector<const char*> giantTextures = {
            "textures/laser_eyes.png",
            "textures/stone.png",
            "textures/stone.png",
            "textures/stone.png"
        };
        std::vector<MaterialProperties> giantProperties = {
            MaterialProperties(  // Grey parts
                    glm::vec3(0.0f),     // No emission
                    1.0f,               // Less shiny
                    false                // Not a light source
                    ),
            MaterialProperties(  // Grey parts
                    glm::vec3(0.0f),     // No emission
                    1.0f,               // Less shiny
                    false                // Not a light source
                    ),
            MaterialProperties(  // Grey parts
                    glm::vec3(0.0f),     // No emission
                    1.0f,               // Less shiny
                    false                // Not a light source
                    ),
            MaterialProperties(  // Grey parts
                    glm::vec3(0.0f),     // No emission
                    1.0f,               // Less shiny
                    false                // Not a light source
                    )
        };
        objects.push_back(new Object(shaderProgram, "models/giant.obj", giantTextures,
                    giantProperties, 90.0f, 0.0f, 0.0f, 1.0f, 1));
    }
    void SetupLighting() {
        // Set up directional light
        // Light source position

        // Assuming we want light to shine towards origin (0,0,0)
        // Calculate direction as: direction = target position - light position
        glm::vec3 targetPos(0.0f, 0.0f, 0.0f);  // Center of your scene
        dirLight.direction = glm::normalize(targetPos - lightPos);

        // The rest remains the same
        dirLight.ambient = glm::vec3(0.2f);     
        dirLight.diffuse = glm::vec3(0.1f);     
        dirLight.specular = glm::vec3(0.5f);   

        // Set light uniforms
        GLint lightDirLoc = glGetUniformLocation(shaderProgram, "dirLight.direction");
        GLint lightAmbientLoc = glGetUniformLocation(shaderProgram, "dirLight.ambient");
        GLint lightDiffuseLoc = glGetUniformLocation(shaderProgram, "dirLight.diffuse");
        GLint lightSpecularLoc = glGetUniformLocation(shaderProgram, "dirLight.specular");

        glUniform3fv(lightDirLoc, 1, glm::value_ptr(dirLight.direction));
        glUniform3fv(lightAmbientLoc, 1, glm::value_ptr(dirLight.ambient));
        glUniform3fv(lightDiffuseLoc, 1, glm::value_ptr(dirLight.diffuse));
        glUniform3fv(lightSpecularLoc, 1, glm::value_ptr(dirLight.specular));
        // Setup point lights
        pointLights.clear();

        // Collect light data from objects
        for (const auto& obj : objects) {
            for (size_t i = 0; i < obj->materials.size(); i++) {
                const auto& mat = obj->materials[i];
                if (mat.isLightSource) {
                    PointLight light;
                    glm::mat4 model = obj->GetModelMatrix();
                    light.position = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                    light.color = mat.emission;
                    light.constant = mat.constant;
                    light.linear = mat.linear;
                    light.quadratic = mat.quadratic;
                    light.ambient = glm::vec3(0.05f);
                    light.diffuse = glm::vec3(0.8f);
                    light.specular = glm::vec3(1.0f);
                    pointLights.push_back(light);
                }
            }
        }

        // Send point light data
        glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), 
                static_cast<GLint>(pointLights.size()));

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
                obj->Rotate(0.003f);
            obj->Draw(polygonal_mode);
        }
        if (showDebugLines) {
            lightVisualizer->Draw(view, projection, lightPos, dirLight.direction, shaderProgram);
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
