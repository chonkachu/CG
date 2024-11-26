// Object.cpp
#include "Object.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Object::Object(GLuint shaderProgram, const char* objPath, 
               const std::vector<const char*>& texturePaths,
               const std::vector<MaterialProperties>& matProperties,
               float _xPos, float _yPos, float _zPos, float _scale, int axis)
    : shaderProgram(shaderProgram), xPos(_xPos), yPos(_yPos), zPos(_zPos),
      scale(_scale), angle(0.0f), axis(axis), name(objPath), materials(matProperties) {
    
    if (!LoadOBJ(objPath)) {
        throw std::runtime_error("Failed to load OBJ file!");
    }

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xPos, yPos, zPos));
    
    glm::vec3 rotation_axis(0.0f, 1.0f, 0.0f);
    if (axis == 0) rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
    if (axis == 2) rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    model = glm::rotate(model, angle, rotation_axis);
    model = glm::scale(model, glm::vec3(scale));

    // Generate and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate and setup VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coord));

    // Normal attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));


    // Load textures
    textures.resize(texturePaths.size());
    for (size_t i = 0; i < texturePaths.size(); i++) {
        if (!LoadTexture(texturePaths[i], textures[i])) {
            throw std::runtime_error("Failed to load texture: " + std::string(texturePaths[i]));
        }
    }
}

bool Object::LoadOBJ(const char* path) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<glm::vec3> temp_normals;
    std::string current_material;
    size_t vertex_count = 0;

    std::ifstream file(path);
    if (!file) {
        std::cerr << "Cannot open file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vt") {
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            temp_texcoords.push_back(tex);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "usemtl") {
            if (!current_material.empty()) {
                materialGroups.back().second.second = vertex_count - materialGroups.back().second.first;
            }
            
            iss >> current_material;
            std::cout << current_material << " vertice inicial = " << vertex_count << std::endl;
            materialGroups.push_back({current_material, {vertex_count, 0}});
        }
        else if (type == "f") {
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;

            auto process_vertex = [&](const std::string& v) {
                std::stringstream ss(v);
                std::string index_str;
                std::vector<int> indices;
                
                while (std::getline(ss, index_str, '/')) {
                    indices.push_back(!index_str.empty() ? std::stoi(index_str) : 0);
                }

                Vertex vertex;
                vertex.position = temp_vertices[indices[0] - 1];
                vertex.texture_coord = indices[1] > 0 ? temp_texcoords[indices[1] - 1] : glm::vec2(0.0f);
                vertex.normal = indices[2] > 0 ? temp_normals[indices[2] - 1] : glm::vec3(0.0f, 1.0f, 0.0f);
                vertices.push_back(vertex);
                vertex_count++;
            };

            process_vertex(v1);
            process_vertex(v2);
            process_vertex(v3);
        }
    }

    if (!current_material.empty()) {
        materialGroups.back().second.second = vertex_count - materialGroups.back().second.first;
    }

    return true;
}

bool Object::LoadTexture(const char* path, GLuint& texture) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    
    if (data) {
        std::cout << path << " ";
        if (channels == 4) {
            std::cout << "RGBA" << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else {
            std::cout << "RGB" << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        stbi_image_free(data);
        return true;
    }
    
    std::cerr << "Failed to load texture: " << path << std::endl;
    return false;
}

void Object::Draw(bool mesh_active) {
    glUseProgram(shaderProgram);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xPos, yPos, zPos));
    
    glm::vec3 rotation_axis(0.0f, 1.0f, 0.0f);
    if (axis == 0) rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
    if (axis == 2) rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    
    model = glm::rotate(model, angle, rotation_axis);
    model = glm::scale(model, glm::vec3(scale));

    GLint loc_model = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(vao);

    for (size_t i = 0; i < materialGroups.size(); i++) {
        GLint emissionLoc = glGetUniformLocation(shaderProgram, "material.emission");
        GLint shininessLoc = glGetUniformLocation(shaderProgram, "material.shininess");
        GLint isLightSourceLoc = glGetUniformLocation(shaderProgram, "material.isLightSource");
        GLint constantLoc = glGetUniformLocation(shaderProgram, "material.constant");
        GLint linearLoc = glGetUniformLocation(shaderProgram, "material.linear");
        GLint quadraticLoc = glGetUniformLocation(shaderProgram, "material.quadratic");
        GLint cutOffLoc = glGetUniformLocation(shaderProgram, "material.cutOff");
        GLint outerCutOffLoc = glGetUniformLocation(shaderProgram, "material.outerCutOff");
        GLint directionLoc = glGetUniformLocation(shaderProgram, "material.direction");

        const auto& mat = materials[i];
        glUniform3fv(emissionLoc, 1, glm::value_ptr(mat.emission));
        glUniform1f(shininessLoc, mat.shininess);
        glUniform1i(isLightSourceLoc, mat.isLightSource);
        glUniform1f(constantLoc, mat.constant);
        glUniform1f(linearLoc, mat.linear);
        glUniform1f(quadraticLoc, mat.quadratic);
        glUniform1f(cutOffLoc, mat.cutOff);
        glUniform1f(outerCutOffLoc, mat.outerCutOff);
        glUniform3fv(directionLoc, 1, glm::value_ptr(mat.direction));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        GLint loc_texture = glGetUniformLocation(shaderProgram, "material.diffuse");
        glUniform1i(loc_texture, 0);

        const auto& group = materialGroups[i];
        glDrawArrays(GL_TRIANGLES, group.second.first, group.second.second);
    }
}

void Object::Move(float dx, float dy) {
    xPos += dx;
    yPos += dy;
}

void Object::Scale(float factor) {
    scale *= factor;
}

void Object::Rotate(float angle_delta) {
    angle += angle_delta;
}

glm::mat4 Object::GetModelMatrix() {
    return model;
}

Object::~Object() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (!textures.empty()) glDeleteTextures(textures.size(), textures.data());
}
