#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

const string resource_path = "../resource/";
const string shaders_path_prefix = resource_path + "shaders/";
const string frag_suffix = "/fragment.glsl";
const string vert_suffix = "/vertex.glsl";
class Shader
{
public:
    unsigned int ID;
    Shader() = default;
    Shader(const string& shader_name) {
	const char* vertexPath = (shaders_path_prefix + shader_name + vert_suffix).c_str();
	const char* fragmentPath= (shaders_path_prefix + shader_name + frag_suffix).c_str();
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode, fragmentCode;
        std::ifstream vShaderFile, fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            vShaderFile.open(vertexPath), fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close(), fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str(), fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str(), *fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    void use() { 
        glUseProgram(ID); 
    }
    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setMat4f(const std::string &name, glm::mat4 value) const { 
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
    }
    void setVec3f(const std::string &name, glm::vec3 value) const { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); 
    }
    
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif