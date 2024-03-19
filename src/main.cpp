#include "config.h"
#include "cloth_mesh.h"
#include <memory>

#include "GLFW/glfw3.h"
#include "algebraic_types.h"
#include "physics_engine.h"
#include "linear_algebra.h"
#include "shader.h"
#include "xpbd_window.h"



// Most of the window and OpenGL setup is based upon GetIntoGameDev's OpenGL tutorial
// at https://www.youtube.com/watch?list=PLn3eTxaOtL2PHxN8EHf-ktAcN-sGETKfw
// and the LearnOpenGL website at https://learnopengl.com/
int main()
{
    XPBDWindow window;
    window.enter_update_loop();



    return 0;
}