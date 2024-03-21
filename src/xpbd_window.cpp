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
        _this->handle_key_input(window, key, scancode, action, mods); });

    // Set the mouse input mode to hide the cursor and keep it centered.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Set handler for the cursor movement.
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos)
                             {
        XPBDWindow *_this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_mouse_input(window, xpos, ypos); });

    initialize_members();
    print_help();
}

/**
 * @brief Closes the window.
 */
XPBDWindow::~XPBDWindow()
{
    glfwTerminate();
}

/**
 * @param window The window for this input.
 * @param xpos The x position of the cursor on the screen.
 * @param ypos The y position of the cursor on the screen.
 *
 * @brief Handle cursor position changes and update camera.
 */
void XPBDWindow::handle_mouse_input(GLFWwindow *window, double xpos, double ypos)
{
    // Calculate offsets. In glfw (0,0) is the upper left corner
    // and y is growing downwards.
    float xoffset = xpos - last_mouse_x;
    float yoffset = last_mouse_y - ypos;
    last_mouse_x = xpos;
    last_mouse_y = ypos;

    // Avoid big jumps when the mouse focuses the window for the first time.
    if (!first_mouse)
        camera.get()->update_angle(xoffset, yoffset);
    first_mouse = false;
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
void XPBDWindow::handle_key_input(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    // Assign movement value for all 3 directions.
    case GLFW_KEY_W:
        if (action == GLFW_PRESS)
            forward += 1;
        else if (action == GLFW_RELEASE)
            forward -= 1;
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS)
            forward -= 1;
        else if (action == GLFW_RELEASE)
            forward += 1;
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS)
            right -= 1;
        else if (action == GLFW_RELEASE)
            right += 1;
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS)
            right += 1;
        else if (action == GLFW_RELEASE)
            right -= 1;
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS)
            up += 1;
        else if (action == GLFW_RELEASE)
            up -= 1;
        break;
    case GLFW_KEY_LEFT_CONTROL:
        if (action == GLFW_PRESS)
            up -= 1;
        else if (action == GLFW_RELEASE)
            up += 1;
        break;
    // Pause / start simulation.
    case GLFW_KEY_P:
        if (action == GLFW_PRESS)
            simulate = !simulate;
        break;
    // Adjust movement speed.
    case GLFW_KEY_RIGHT_BRACKET:
        if (action == GLFW_PRESS)
        {
            camera.get()->movement_speed *= 10.f;
            std::cout << "new camera speed: " << camera.get()->movement_speed << std::endl;
        }
        break;
    case GLFW_KEY_BACKSLASH:
        if (action == GLFW_PRESS)
        {
            camera.get()->movement_speed /= 10.f;
            std::cout << "new camera speed: " << camera.get()->movement_speed << std::endl;
        }
        break;
    // Reset the cloth.
    case GLFW_KEY_R:
        if (action == GLFW_PRESS)
        {
            reset_cloth();
        }
        break;
    // Print the help text.
    case GLFW_KEY_H:
        if (action == GLFW_PRESS)
            print_help();
        break;
    }
}

void XPBDWindow::print_help()
{
    std::cout << "==============================" << std::endl;
    std::cout << "  Keyboard bindings:" << std::endl;
    std::cout << "h: print this help" << std::endl;
    std::cout << "w: move forward" << std::endl;
    std::cout << "a: move left" << std::endl;
    std::cout << "s: move backwards" << std::endl;
    std::cout << "d: move right" << std::endl;
    std::cout << "+: increase camera speed" << std::endl;
    std::cout << "#: decrease camera speed" << std::endl;
    std::cout << "p: pause simulation" << std::endl;
    std::cout << "r: reset the experiment" << std::endl;

    std::cout << "==============================" << std::endl;
}

void XPBDWindow::reset_cloth()
{
    // Create the cloth and give it a color.
    vec3 color = {1.0f, 0.0f, 0.0f};

    cloth = std::make_unique<ClothMesh>("../assets/cloth_1.obj", color);

    float3 gravity;
    gravity.data[0] = 0.f;
    gravity.data[1] = -0.00001f;
    gravity.data[2] = 0.f;

    cloth_physics = std::make_unique<PhysicsEngine>(cloth.get(), gravity);
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
    // Enable depth testing. This will disregard fragments (pixels)
    // that are behind other pixels.
    glEnable(GL_DEPTH_TEST);

    // Set the background color of the window.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Set up the cloth in the scene.
    reset_cloth();

    delta_time = 0.0f;
    last_frame = 0.0f;

    float3 gravity;
    gravity.data[0] = 0.f;
    gravity.data[1] = -0.00001f;
    gravity.data[2] = 0.f;

    cloth_physics = std::make_unique<PhysicsEngine>(cloth.get(), gravity);
    simulate = false;

    // Create a shader for the objects in the scene.
    shader = std::make_unique<Shader>("shaders/vertex.txt", "shaders/fragment.txt");

    // Create a camera at the given position.
    camera = std::make_unique<Camera>(vec3{0.0f, 0.0f, 1.0f});

    // Determine the model matrix for the cloth rotation and translation.
    position = {-0.5f, -0.5f, 0.0f};
    rotation = {0.0f, 0.0f, 0.0f};
    model_matrix = model(position, rotation, 1.0f);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    first_mouse = true;
    last_mouse_x = width / 2.0f;
    last_mouse_y = height / 2.0f;

    forward = 0;
    right = 0;
    up = 0;

    float aspect = width / (float)height;

    // Create a projection matrix with set fov, and near and far distance limits.
    projection_matrix = projection(camera.get()->fov, aspect, 0.1f, 200.0f);
}

/**
 * @brief Update the scene with the current changes.
 */
void XPBDWindow::update_window()
{
    // Calculate frame time to allow for fps independent movement.
    double curr_frame = glfwGetTime();
    delta_time = curr_frame - last_frame;
    last_frame = curr_frame;

    // Update the camera based on the movement.
    camera.get()->update_movement(forward, right, up, delta_time);

    // Update the view matrix
    view_matrix = camera.get()->get_view();

    if (simulate)
    {
        // Implements the physics engine.
        cloth_physics->update();
    }

    // Clear the background color buffer.
    // This sets the color to the one defined by glClearColor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader in the new buffer.
    shader->use();

    // Set uniform shader variables.
    glUniformMatrix4fv(3, 1, GL_FALSE, model_matrix.entries);
    glUniformMatrix4fv(4, 1, GL_FALSE, view_matrix.entries);
    glUniformMatrix4fv(5, 1, GL_FALSE, projection_matrix.entries);
    glUniform3fv(6, 1, camera.get()->position.entries);
    glUniform3f(7, -5.0f, 5.0f, 5.0f); // light_pos
    glUniform3f(8, 1.0f, 1.0f, 1.0f);  // light_color
    glUniform1f(9, 0.5f);              // ambient_strength
    glUniform1f(10, 0.1f);             // specular_strength

    // Draw the cloth onto the screen.
    cloth->draw();

    // Gives the window the new buffer updated with glClear.
    glfwSwapBuffers(window);

    // Poll and consume all events for this frame.
    glfwPollEvents();
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
