#include "xpbd_window.h"
#include <cassert>

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
    // Handle window resizing
    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height)
                              {
        XPBDWindow *_this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_window_resize(window, width, height); });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
                                   {
        XPBDWindow *_this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_buffer_resize(window, width, height); });
    glfwSetWindowRefreshCallback(window, [](GLFWwindow *window)
                                 {
        XPBDWindow *_this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
        _this->handle_window_refresh(window); });

    // Handle keyboard inputs
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

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods)
                               {
            XPBDWindow* _this = static_cast<XPBDWindow*>(glfwGetWindowUserPointer(window));
            _this->handle_mouse_button_input(window, button, action, mods); });

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
 * @param window The window for this event.
 * @param width The new width in screen coordinates.
 * @param height The new height in screen coordinates.
 *
 * @brief Updates the projection matrix to the new window size.
 */
void XPBDWindow::handle_window_resize(GLFWwindow *window, int width, int height)
{
    (void)window;

    float aspect = width / (float)height;

    // Update projection matrix with new aspect ratio.
    projection_matrix = projection(camera->fov, aspect, 0.1f, 200.0f);
}

/**
 * @param window The window for this event.
 * @param width The new width in pixels.
 * @param height The new height in pixels.
 *
 * @brief Updates the OpenGL viewport size to the new window size.
 */
void XPBDWindow::handle_buffer_resize(GLFWwindow *window, int width, int height)
{
    (void)window;

    glViewport(0, 0, width, height);
}

/**
 * @brief Draw the window while resizing.
 */
void XPBDWindow::handle_window_refresh(GLFWwindow *window)
{
    render();
    glfwSwapBuffers(window);
    glFinish();
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
    (void)window;

    if (!mouse_input_enabled)
        return;
    // Calculate offsets. In glfw (0,0) is the upper left corner
    // and y is growing downwards.
    float xoffset = xpos - last_mouse_x;
    float yoffset = last_mouse_y - ypos;
    last_mouse_x = xpos;
    last_mouse_y = ypos;

    // Avoid big jumps when the mouse focuses the window for the first time.
    if (!first_mouse)
        camera->update_angle(xoffset, yoffset);
    first_mouse = false;
}

/**
 * @brief compute user input for mouse button event and translate it into camera movement.
 *
 * @param window The glfw window associated with this simulation window.
 * @param button The button associated with the event.
 * @param action The event being triggered.
 * @param mods The modifier keys being pressed.
 */
void XPBDWindow::handle_mouse_button_input(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        if (action == GLFW_PRESS)
        {
            mouse_input_enabled = true;
            first_mouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        break;
    }
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
    (void)window;
    (void)scancode;
    (void)mods;

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
            camera->movement_speed *= 10.f;
            std::cout << "new camera speed: " << camera->movement_speed << std::endl;
        }
        break;
    case GLFW_KEY_BACKSLASH:
        if (action == GLFW_PRESS)
        {
            camera->movement_speed /= 10.f;
            std::cout << "new camera speed: " << camera->movement_speed << std::endl;
        }
        break;
        // Reset the cloth.
    case GLFW_KEY_R:
        if (action == GLFW_PRESS)
        {
            reset_cloth();
        }
        break;
    case GLFW_KEY_1:
    case GLFW_KEY_2:
    case GLFW_KEY_3:
    case GLFW_KEY_4:
        if (action == GLFW_PRESS)
        {
            mounting_type = static_cast<MountingType>(key);
            reset_cloth();
        }
        break;
    case GLFW_KEY_F1:
    case GLFW_KEY_F2:
    case GLFW_KEY_F3:
    case GLFW_KEY_F4:
    case GLFW_KEY_F5:
    case GLFW_KEY_F6:
        if (action == GLFW_PRESS)
        {
            mesh_id = key;
            reset_cloth();
        }
        break;

        // Print the help text.
    case GLFW_KEY_H:
        if (action == GLFW_PRESS)
            print_help();
        break;
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
        {
            mouse_input_enabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        break;
    case GLFW_KEY_F:
        if (action == GLFW_PRESS)
        {
            if (draw_wire_frame)
            {
                draw_wire_frame = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                draw_wire_frame = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        break;
    }
}

/**
 * @brief Print the help text for the simulation.
 */
void XPBDWindow::print_help()
{
    std::cout << "==============================" << std::endl;
    std::cout << "  Keyboard bindings:" << std::endl;
    std::cout << "h:   print this help" << std::endl;
    std::cout << "w:   move forward" << std::endl;
    std::cout << "a:   move left" << std::endl;
    std::cout << "s:   move backwards" << std::endl;
    std::cout << "d:   move right" << std::endl;
    std::cout << "+:   increase camera speed" << std::endl;
    std::cout << "#:   decrease camera speed" << std::endl;
    std::cout << "p:   pause simulation" << std::endl;
    std::cout << "r:   reset the experiment" << std::endl;
    std::cout << "f:   toggle wireframe" << std::endl;
    std::cout << "ESC: free the mouse" << std::endl;

    std::cout << "   ---MOUNTING METHODS---" << std::endl
              << "1: top right corner" << std::endl
              << "2: top row" << std::endl
              << "3: middle vertex" << std::endl
              << "4: none" << std::endl
              << "   ---MOUNTING METHODS---" << std::endl;

    std::cout << "   ---MESH RESOLUTIONS---" << std::endl
              << "F1: 10x10" << std::endl
              << "F2: 25x25" << std::endl
              << "F3: 50x50" << std::endl
              << "F4: 50x50 (no noise)" << std::endl
              << "F5: 100x100" << std::endl
              << "F6: 200x200" << std::endl
              << "   ---MESH RESOLUTIONS---" << std::endl;

    std::cout << "==============================" << std::endl;
}

/**
 * @brief Reset the cloth simulation.
 */
void XPBDWindow::reset_cloth()
{
    // Create the cloth and give it a color.
    vec3 color = {1.0f, 0.0f, 0.0f};
    switch (mesh_id)
    {
    case GLFW_KEY_F1:
        cloth = std::make_unique<ClothMesh>("assets/cloth_10.obj", color);
        break;
    case GLFW_KEY_F2:
        cloth = std::make_unique<ClothMesh>("assets/cloth_25.obj", color);
        break;
    case GLFW_KEY_F3:
        cloth = std::make_unique<ClothMesh>("assets/cloth_50.obj", color);
        break;
    case GLFW_KEY_F4:
        cloth = std::make_unique<ClothMesh>("assets/cloth_50_smooth.obj", color);
        break;
    case GLFW_KEY_F5:
        cloth = std::make_unique<ClothMesh>("assets/cloth_100.obj", color);
        break;
    case GLFW_KEY_F6:
        cloth = std::make_unique<ClothMesh>("assets/cloth_200.obj", color);
        break;
    default:
        assert(false);
        std::exit(42);
        break;
    }

    // Determine the model matrix for the cloth rotation and translation.
    position = {0.0f, 0.0f, 0.0f};
    rotation = {0.0f, 0.0f, 0.0f};
    model_matrix = model(position, rotation, 1.0f);

    vec3 gravity;
    gravity.entries[0] = 0.f;
    gravity.entries[1] = -9.81f;
    gravity.entries[2] = 0.f;

    MountingType m = static_cast<MountingType>(mounting_type);

#ifdef USE_CONCURRENT_PHYSICS_ENGINE
    cloth_physics = std::make_unique<ConcurrentPhysicsEngine>(cloth.get(), gravity, m);
#else
    cloth_physics = std::make_unique<PhysicsEngine>(cloth.get(), gravity, m);
#endif
}

/**
 * @brief Initializes the static setup for OpenGL.
 */
void XPBDWindow::initialize_members()
{
    // Enable face culling. This will assume a counter
    // clockwise definition of triangles in the vertex buffer
    // and culls the backfacing side of the triangle.
    // glEnable(GL_CULL_FACE);

    // Enable depth testing. This will disregard fragments (pixels)
    // that are behind other pixels.
    glClearDepth(0.0f);
    glDepthFunc(GL_GEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Set the background color of the window.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mounting_type = MountingType::CORNER_VERTEX;
    mesh_id = GLFW_KEY_F3;

    // Set up the cloth in the scene.
    reset_cloth();

    delta_time = 0.0f;
    last_frame = 0.0f;

    simulate = false;
    draw_wire_frame = true;

    // Create a shader for the objects in the scene.
    shader = std::make_unique<Shader>("shaders/vertex.txt", "shaders/fragment.txt");

    // Create a camera at the given position.
    camera = std::make_unique<Camera>(vec3{0.0f, 0.0f, 1.0f});

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
    projection_matrix = projection(camera->fov, aspect, 0.1f, 200.0f);
}

void XPBDWindow::render()
{
    // Calculate frame time to allow for fps independent movement.
    double curr_frame = glfwGetTime();
    delta_time = curr_frame - last_frame;
    last_frame = curr_frame;

    if (curr_frame - last_fps_print >= 1.f)
    {
        std::stringstream ss;
        ss << "XPBD Cloth simulation FPS: " << 1 / delta_time;
        glfwSetWindowTitle(window, ss.str().c_str());
        last_fps_print = curr_frame;
    }

    // Update the camera based on the movement.
    camera->update_movement(forward, right, up, delta_time);

    // Update the view matrix
    view_matrix = camera->get_view();

    // Clear the background color buffer.
    // This sets the color to the one defined by glClearColor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader in the new buffer.
    shader->use();

    // Set uniform shader variables.
    glUniformMatrix4fv(3, 1, GL_FALSE, model_matrix.entries);
    glUniformMatrix4fv(4, 1, GL_FALSE, view_matrix.entries);
    glUniformMatrix4fv(5, 1, GL_FALSE, projection_matrix.entries);
    glUniform3fv(6, 1, camera->position.entries);
    glUniform3f(7, -10.0f, 0.0f, 10.0f); // light_pos
    glUniform3f(8, 1.0f, 1.0f, 1.0f);    // light_color
    glUniform1f(9, 0.5f);                // ambient_strength
    glUniform1f(10, 0.1f);               // specular_strength
}

/**
 * @brief Update the scene with the current changes.
 */
void XPBDWindow::update_window()
{
    render();

#ifdef USE_CONCURRENT_PHYSICS_ENGINE
    if (simulate)
    {
        cloth_physics->wait();
    }
#endif

    // Draw the cloth onto the screen.
    cloth->draw();

    if (simulate)
    {
        // Implements the physics engine.
        cloth_physics->update();
    }

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
