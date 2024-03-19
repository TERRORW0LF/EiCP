#pragma once
#include "string"


class Shader {
public:
	Shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
	Shader(Shader& other) = delete;

	Shader operator=(Shader& other) = delete;

	~Shader();

	void use();

private:
	unsigned int shader;

	unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
	unsigned int make_module(const std::string& filepath, unsigned int module_type);
};