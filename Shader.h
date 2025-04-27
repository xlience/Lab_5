#ifndef SHADER_H // Проверяем, был ли ранее определён этот заголовок
#define SHADER_H

#include <GL/glew.h>
#include "glm.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glew-2.1.0/glew-2.1.0/include/GL/glew.h"

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode = loadFile(vertexPath);
        std::string fragmentCode = loadFile(fragmentPath);

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        GLuint vertex, fragment;
        vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
        fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);

        checkProgramLinking(ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() {
        glUseProgram(ID);
    }

    void setFloat(const std::string& name, float value) {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setInt(const std::string& name, int value) {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string& name, float x, float y) {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }

    void setVec3(const std::string& name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void setVec4(const std::string& name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

    void setMat4(const std::string& name, const float* value) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
    }

    void setMat3(const std::string& name, const glm::mat3& value) {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

private:
    std::string loadFile(const char* path) {
        std::ifstream file(path);
        std::stringstream stream;

        if (!file.is_open()) {
            std::cerr << "Could not open file: " << path << std::endl;
            return "";
        }

        stream << file.rdbuf();
        return stream.str();
    }

    GLuint compileShader(GLenum type, const char* code) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &code, nullptr);
        glCompileShader(shader);

        GLint success;
        GLchar infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "Shader Compilation Error: " << infoLog << std::endl;
        }

        return shader;
    }

    void checkProgramLinking(GLuint program) {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 1024, nullptr, infoLog);
            std::cerr << "Program Linking Error: " << infoLog << std::endl;
        }
    }
};

#endif // SHADER_H
