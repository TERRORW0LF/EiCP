#include "xpbd_window.h"


XPBDWindow::XPBDWindow()
{    
    // The current and only window used in this application.

    // Initialize the window management framework glfw.
    // If initialization fails, abort execution with error code.
    if (!glfwInit())
    {
        std::cout << "GLFW couldn't be initialized";
        std::exit(-1);
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
        std::exit(-1);
    }

    //https://subscription.packtpub.com/book/business-and-other/9781803246529/2/ch02lvl1sec06/event-handling-in-glfw
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto _this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_input(window, key, scancode, action, mods);
        });

    initialize_members();
}

XPBDWindow::~XPBDWindow()
{
    glfwTerminate();
}

void XPBDWindow::handle_input(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    std::cout << "input " << key << std::endl;
}

void XPBDWindow::initialize_members()
{
    // Set the background color of the window.
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);

    // Create the cloth and give it a color.
    float color[3] = { 1.0f, 0.0f, 0.0f };

    cloth = std::make_unique<ClothMesh>("../assets/cloth_1.obj", color);

    float3 gravity;
    gravity.data[0] = 0.f;
    gravity.data[1] = -0.00001f;
    gravity.data[2] = 0.f;

    clothPhysics = std::make_unique<PhysicsEngine>(cloth.get(), gravity);

    // Create a shader for the objects in the scene.
    shader = std::make_unique<Shader>("../src/shaders/vertex.txt", "../src/shaders/fragment.txt");


    // Determine the model matrix for the cloth rotation and translation.
    cloth_position = { -0.5f, 0.9f, 0.0f };
    cloth_rotation = { 0.0f, 0.0f, 0.0f };
    model_matrix = model(cloth_position, cloth_rotation, 1.0f);

    // Set the camera position and calculate the view transform.
    camera_pos = { 0.1f, 0.1f, 0.2f };
    camera_target = { 0.0f, 0.0f, 0.0f };
    view_matrix = view(camera_pos, camera_target);
}

void XPBDWindow::update_window()
{




    // Poll all events for this frame.
    glfwPollEvents();

    // Clear the background color buffer.
    // This sets the color to the one defined by glClearColor.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the shader in the new buffer.
    shader->use();

    // Set uniform shader variables.
    glUniformMatrix4fv(3, 1, GL_FALSE, model_matrix.entries);
    glUniformMatrix4fv(4, 1, GL_FALSE, view_matrix.entries);
    glUniform3f(5, -1.0, 0.0, 5.0);//lightPosition
    glUniform3f(6, 1.0, 1.0, 1.0);//lightColor
    glUniform1f(7, 0.05);//ambientStrength

    // Draw the cloth onto the screen.
    cloth->draw();

    // Gives the window the new buffer updated with glClear.
    glfwSwapBuffers(window);

    // Implements the physics engine.
    clothPhysics->update();
}

void XPBDWindow::enter_update_loop()
{
    while (!glfwWindowShouldClose(window))
    {
        update_window();
    }
}
