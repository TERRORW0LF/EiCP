#include "config.h"
#include "cloth_mesh.h"
#include <memory>

#include "GLFW/glfw3.h"
#include "algebraic_types.h"

// Declare function prototypes to avoid sorting them in code.
unsigned int make_shader(const std::string &vertex_filepath, const std::string &fragment_filepath);
unsigned int make_module(const std::string &filepath, unsigned int module_type);

// Most of the window and OpenGL setup is based upon GetIntoGameDev's OpenGL tutorial
// at https://www.youtube.com/watch?list=PLn3eTxaOtL2PHxN8EHf-ktAcN-sGETKfw
int main()
{
    // The current and only window used in this application.
    GLFWwindow *window;

    // Initialize the window management framework glfw.
    // If initialization fails, abort execution with error code.
    if (!glfwInit())
    {
        std::cout << "GLFW couldn't be initialized";
        return -1;
    }

    // Tell glfw which version of OpenGL we're using.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the window and set it as the currently active window.
    window = glfwCreateWindow(640, 480, "XPBD Cloth simulation", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Dynamically load the OpenGL libraries.
    // This is done via glad, generated for the core OpenGL functionality
    // and OpenGL version 4.6.
    // Abort with error code if OpenGL libraries couldn't be found.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    // Set the background color of the window.
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);

    // Create the cloth and give it a color.
    float color[3] = {1.0f, 0.0f, 0.0f};
    std::unique_ptr<ClothMesh> cloth = std::make_unique<ClothMesh>("../assets/cloth_1.obj", color);

    // Create a shader for the objects in the scene.
    unsigned int shader = make_shader("../src/shaders/vertex.txt", "../src/shaders/fragment.txt");

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    RealVector<int, 5> test;
    RealVector<int, 5> test2;

    test + test2;



    // Window event loop. Runs until the user closes the window.
    while (!glfwWindowShouldClose(window))
    {
        // Poll all events for this frame.
        glfwPollEvents();

        // Clear the background color buffer.
        // This sets the color to the one defined by glClearColor.
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader in the new buffer.
        glUseProgram(shader);

        // Draw the cloth onto the screen.
        cloth->draw();

        // Gives the window the new buffer updated with glClear.
        glfwSwapBuffers(window);


        //update physics here!

        std::vector<float3> vertices = cloth->get_vertex_positions();
        for (float3 & v : vertices) {
            v.data[0] += 0.001f;
            v.data[1] += 0.001f;
            v.data[2] += 0.001f;
        }
        cloth->set_vertex_positions(vertices);


    }
    // Delete shader program before terminating.
    glDeleteProgram(shader);

    // Close the window and return ok status code.
    glfwTerminate();
    return 0;
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
unsigned int make_shader(const std::string &vertex_filepath, const std::string &fragment_filepath)
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
unsigned int make_module(const std::string &filepath, unsigned int module_type)
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
    const char *shaderSrc = shaderSource.c_str();
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
