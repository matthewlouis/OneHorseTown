#pragma once

#include <GL/glew.h>
#include <string>


class GLSLProgram
{
public:
	GLSLProgram();
	~GLSLProgram();

	void compileShaders(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

	void linkShaders();

	void addAttribute(const std::string& attributeName);

	void use();
	void unUse();

private:
	void compileShader(const std::string& filePath, GLuint& id);
	
	int _numAttributes;
	GLuint _programID;
	GLuint _vertexShaderID;
	GLuint _fragmentShaderID;
};

