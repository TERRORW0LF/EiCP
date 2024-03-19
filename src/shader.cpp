#include "shader.h"
#include "config.h"

Shader::Shader(const std::string& vertex_filepath, const std::string& fragment_filepath)
{
    shader = make_shader(vertex_filepath, fragment_filepath);
}

Shader::~Shader()
{
    glDeleteProgram(shader);
}

void Shader::use()
{
    glUseProgram(shader);
}

unsigned int Shader::make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath)
{
    // Create a shader and relevant shader modules.
    unsigned int shader = glCreateProgram();
    unsigned int vertex_shader = make_module(vertex_filepath, GL_VERTEX_SHADER);
    unsigned int fragment_shader = make_module(fragment_filepath, GL_FRAGMENT_SHADER);

    // Link modules to shader.
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);

    // Check linking status and print error if linking failed.
    // This might throw a segfault if errorLog is longer than 1024 chars.
    int success;
    glGetShaderiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "Shader linking error:\n"
            << errorLog << std::endl;
    }

    // Delete individual shader modules as we don't need them after linking.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader;
}

unsigned int Shader::make_module(const std::string& filepath, unsigned int module_type)
{
    // Setup streams for reading shader module file.
    std::ifstream file;
    std::stringstream bufferedLines;
    std::string line;

    // Try to open the shader file. Print warning if that failed.
    file.open(filepath);
    if (!file.is_open())
    {
        std::cout << "Unable to open shader file." << std::endl;
    }

    // Read file and convert it into a char array.
    while (std::getline(file, line))
    {
        bufferedLines << line << "\n";
    }

    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();
    file.close();

    // Compile shader module
    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);

    // Check compilation status and print error if compilation failed.
    // This might throw a segfault if errorLog is longer than 1024 chars.
    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "Shader Module compilation error:\n"
            << errorLog << std::endl;
    }

    return shaderModule;
}
