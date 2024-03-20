#include "xpbd_window.h"

/**
 * This function creates a glfw window and sets up the XPBD cloth simulation.
 * This class handles rendering and window inputs.
 *
 * @returns A window with the simulation.
 * @brief Creates a window and initializes it with our simulation.
 */
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

    // https://subscription.packtpub.com/book/business-and-other/9781803246529/2/ch02lvl1sec06/event-handling-in-glfw
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
        auto _this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_input(window, key, scancode, action, mods); });

    initialize_members();
}

/**
 * @brief Closes the window.
 */
XPBDWindow::~XPBDWindow()
{
    glfwTerminate();
}

/**
 * Handel keyboard inputs and translate them into player movement.
 *
 * @param window The glfw window associated with this simulation window.
 * @param key The key associated with the event.
 * @param scancode The scancode of the key associated with the event.
 * @param action The event being triggered.
 * @param mods The modifier keys being pressed.
 *
 * @brief Handles keyboard inputs.
 */
void XPBDWindow::handle_input(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_W:
        if (action == GLFW_PRESS)
            *camera_movement.get() += *camera_front.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() -= *camera_front.get();
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS)
            *camera_movement.get() -= *camera_front.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() += *camera_front.get();
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS)
            *camera_movement.get() -= *camera_right.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() += *camera_right.get();
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS)
            *camera_movement.get() += *camera_right.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() -= *camera_right.get();
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS)
            *camera_movement.get() += *camera_up.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() -= *camera_up.get();
        break;
    case GLFW_KEY_LEFT_CONTROL:
        if (action == GLFW_PRESS)
            *camera_movement.get() -= *camera_up.get();
        else if (action == GLFW_RELEASE)
            *camera_movement.get() += *camera_up.get();
        break;
    case GLFW_KEY_P:
        if (action == GLFW_PRESS)
            simulate = !simulate;
        break;

    default:
        std::cout << "key " << key << " action " << action << std::endl;
    }
}

void XPBDWindow::update_camera()
{
    const float camera_speed = 0.0003;
    *camera_pos.get() += *camera_movement.get() * camera_speed;
    view_update = true;
    return;
}

/**
 * @brief Initializes the static setup for OpenGL.
 */
void XPBDWindow::initialize_members()
{
    // Enable face culling. This will assume a counter
    // clockwise definition of triangles in the vertex buffer
    // and culls the backfacing side of the triangle.
    glEnable(GL_CULL_FACE);

    // Set the background color of the window.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create the cloth and give it a color.
    float color[3] = {1.0f, 0.0f, 0.0f};

    cloth = std::make_unique<ClothMesh>("assets/cloth_1.obj", color);

    float3 gravity;
    gravity.data[0] = 0.f;
    gravity.data[1] = -0.00001f;
    gravity.data[2] = 0.f;

    cloth_physics = std::make_unique<PhysicsEngine>(cloth.get(), gravity);
    simulate = false;

    // Create a shader for the objects in the scene.
    shader = std::make_unique<Shader>("shaders/vertex.txt", "shaders/fragment.txt");

    // Determine the model matrix for the cloth rotation and translation.
    position = std::make_unique<vec3>((vec3){-0.5f, -0.5f, 0.0f});
    rotation = std::make_unique<vec3>((vec3){0.0f, 0.0f, 0.0f});
    model_matrix = std::make_unique<mat4>((mat4)model(*position.get(), *rotation.get(), 1.0f));

    // Set the camera position and calculate the view transform.
    camera_movement = std::make_unique<vec3>((vec3){0.0f, 0.0f, 0.0f});
    camera_pos = std::make_unique<vec3>((vec3){0.5f, -0.25f, 2.0f});
    camera_front = std::make_unique<vec3>((vec3){0.0f, 0.0f, -1.0f});
    camera_up = std::make_unique<vec3>((vec3){0.0f, 1.0f, 0.0f});
    camera_right = std::make_unique<vec3>((vec3){1.0, 0.0f, 0.0f});
    view_matrix = std::make_unique<mat4>((mat4)view(*camera_pos.get(), *camera_front.get(), *camera_up.get(), *camera_right.get()));
    view_update = true;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect = width / (float)height;
    // Create a projection matrix with set fov, and near and far distance limits.
    projection_matrix = std::make_unique<mat4>((mat4)projection(103.0f, aspect, 0.5f, 200.0f));
}

/**
 * @brief Update the scene with the current changes.
 */
void XPBDWindow::update_window()
{

    // Poll and consume all events for this frame.
    glfwPollEvents();

    // Update the camera based on the movement.
    update_camera();

    // Update the view matrix
    if (view_update)
        view_matrix = std::make_unique<mat4>((mat4)view(*camera_pos.get(), *camera_front.get(), *camera_up.get(), *camera_right.get()));
    view_update = false;

    // Clear the background color buffer.
    // This sets the color to the one defined by glClearColor.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the shader in the new buffer.
    shader->use();

    // Set uniform shader variables.
    glUniformMatrix4fv(3, 1, GL_FALSE, model_matrix.get()->entries);
    glUniformMatrix4fv(4, 1, GL_FALSE, view_matrix.get()->entries);
    glUniformMatrix4fv(5, 1, GL_FALSE, projection_matrix.get()->entries);
    glUniform3fv(6, 1, camera_pos.get()->entries);
    glUniform3f(7, -5.0f, 5.0f, 5.0f); // light_pos
    glUniform3f(8, 1.0f, 1.0f, 1.0f);  // light_color
    glUniform1f(9, 0.05f);             // ambient_strength
    glUniform1f(10, 0.8f);             // specular_strength

    // Draw the cloth onto the screen.
    cloth->draw();

    // Gives the window the new buffer updated with glClear.
    glfwSwapBuffers(window);

    if (simulate)
    {
        // Implements the physics engine.
        cloth_physics->update();
    }
}

/**
 * @brief Start the event loop to update the window.
 */
void XPBDWindow::enter_update_loop()
{
    while (!glfwWindowShouldClose(window))
    {
        update_window();
    }
}
