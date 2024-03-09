#include "config.h"

// Most of the window setup is based upon GetIntoGameDev's OpenGL tutorial
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

    // Window event loop. Runs until the user closes the window.
    while (!glfwWindowShouldClose(window))
    {
        // Poll all events for this frame.
        glfwPollEvents();

        // Clear the background color buffer.
        // This sets the color to the one defined by glClearColor.
        glClear(GL_COLOR_BUFFER_BIT);

        // Gives the window the new buffer updated with glClear.
        glfwSwapBuffers(window);
    }

    // Close the window and return ok status code.
    glfwTerminate();
    return 0;
}
