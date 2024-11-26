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
};

class Object {
public:
    std::string name;
    Object(GLuint shaderProgram, const char* objPath, 
           const std::vector<const char*>& texturePaths,
           float _xPos = 0.0f, float _yPos = 0.0f, float _zPos = 0.0f, 
           float _scale = 1.0f, int axis = 1);
    ~Object();
    
    void Draw(bool mesh_active);
    void Move(float dx, float dy);
    void Scale(float factor);
    void Rotate(float angle);

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
