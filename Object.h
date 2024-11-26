// Object.h
#ifndef OBJ_H
#define OBJ_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texture_coord;
    glm::vec3 normal;
};


// In Object.h
struct MaterialProperties {
    glm::vec3 emission{0.0f};
    float shininess{32.0f};
    bool isLightSource{false};
    float constant{1.0f};
    float linear{0.09f};
    float quadratic{0.032f};
    float cutOff{-1.0f};         // Default to -1 for point lights
    float outerCutOff{-1.0f};    // Default to -1 for point lights
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    MaterialProperties(
        glm::vec3 _emission = glm::vec3(0.0f),
        float _shininess = 32.0f,
        bool _isLightSource = false,
        float _constant = 1.0f,
        float _linear = 0.09f,
        float _quadratic = 0.032f,
        float _cutOff = -1.0f,
        float _outerCutOff = -1.0f,
        glm::vec3 _direction = glm::vec3(0.0f, 0.0f, -1.0f)
    ) : emission(_emission),
        shininess(_shininess),
        isLightSource(_isLightSource),
        constant(_constant),
        linear(_linear),
        quadratic(_quadratic),
        cutOff(_cutOff),
        outerCutOff(_outerCutOff),
        direction(_direction)
    {}
};

class Object {
public:
    std::string name;
    std::vector<MaterialProperties> materials;
    glm::mat4 model;

    Object(GLuint shaderProgram, const char* objPath, 
           const std::vector<const char*>& texturePaths,
           const std::vector<MaterialProperties>& matProperties,
           float _xPos = 0.0f, float _yPos = 0.0f, float _zPos = 0.0f, 
           float _scale = 1.0f, int axis = 1);
    ~Object();
    
    void Draw(bool mesh_active);
    void Move(float dx, float dy);
    void Scale(float factor);
    void Rotate(float angle);
    glm::mat4 GetModelMatrix();
private:
    GLuint vao{0}, vbo{0};
    GLuint shaderProgram;
    std::vector<Vertex> vertices;
    std::vector<GLuint> textures;
    std::vector<std::pair<std::string, std::pair<size_t, size_t>>> materialGroups; // name, {start, count}
    float xPos, yPos, zPos, scale, angle;
    int axis;

    bool LoadOBJ(const char* path);
    bool LoadTexture(const char* path, GLuint& texture);
};

#endif
