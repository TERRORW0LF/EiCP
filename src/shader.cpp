#include "shader.h"
#include "config.h"

/**
 * @param vertex_filepath The relative file path to the vertex shader.
 * @param fragment_filepath The relative file path to the fragment shader.
 * @returns A shader.
 *
 * @brief Creates a shader with the given modules.
 */
Shader::Shader(const std::string &vertex_filepath, const std::string &fragment_filepath)
{
    shader = make_shader(vertex_filepath, fragment_filepath);
}

/**
 * @brief Delete the shader.
 */
Shader::~Shader()
{
    glDeleteProgram(shader);
}

/**
 * @brief Use the shader for the upcoming render.
 */
void Shader::use()
{
    glUseProgram(shader);
}

/**
 * Creates an OpenGL shader from given file paths.
 * The file paths must be relative to the current working directory.
 *
 * @param vertex_filepath The path to the vertex shader file.
 * @param fragement_filepath The path to the fragment shader file.
 * @returns an OpenGL shader.
 *  @brief Creates an OpenGL shader from a given filepath.
 */
unsigned int Shader::make_shader(const std::string &vertex_filepath, const std::string &fragment_filepath)
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
        char error_log[1024];
        glGetProgramInfoLog(shader, 1024, NULL, error_log);
        std::cout << "Shader linking error:\n"
                  << error_log << std::endl;
    }

    // Delete individual shader modules as we don't need them after linking.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader;
}

/**
 * Creates an OpenGL shader module from a given file path.
 * The file path must be relative to the current working directory.
 * If the shader compilation fails a warning will be logged and the
 * faulty module returned.
 *
 * @param filepath The path to the shader file.
 * @param module_type The type of shader module to create.
 * @returns an OpenGL shader module.
 *  @brief Creates an OpenGL shader module from a given filepath.
 */
unsigned int Shader::make_module(const std::string &filepath, unsigned int module_type)
{
    // Setup streams for reading shader module file.
    std::ifstream file;
    std::stringstream buffered_lines;
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
        buffered_lines << line << "\n";
    }

    std::string shader_source = buffered_lines.str();
    const char *shader_src = shader_source.c_str();
    file.close();

    // Compile shader module
    unsigned int shader_module = glCreateShader(module_type);
    glShaderSource(shader_module, 1, &shader_src, NULL);
    glCompileShader(shader_module);

    // Check compilation status and print error if compilation failed.
    // This might throw a segfault if error_log is longer than 1024 chars.
    int success;
    glGetShaderiv(shader_module, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char error_log[1024];
        glGetShaderInfoLog(shader_module, 1024, NULL, error_log);
        std::cout << "Shader Module compilation error:\n"
                  << error_log << std::endl;
    }

    return shader_module;
}
