#pragma once

#include "config.h"
#include "cloth_mesh.h"
#include "physics_engine.h"
#include "shader.h"
#include "linear_algebra.h"
#include "camera.h"

class XPBDWindow
{
public:
	XPBDWindow();
	~XPBDWindow();

private:
	GLFWwindow *window;

	void handle_key_input(GLFWwindow *window, int key, int scancode, int action, int mods);
	void handle_mouse_input(GLFWwindow *window, double xpos, double ypos);

	void initialize_members();

	void print_help();
	void reset_cloth();

public:
	void update_window();
	void update_camera();
	// blocking!
	void enter_update_loop();

	// current members:
private:
	std::unique_ptr<ClothMesh> cloth;
	std::unique_ptr<PhysicsEngine> cloth_physics;
	std::unique_ptr<Shader> shader;
	std::unique_ptr<Camera> camera;

	double delta_time;
	double last_frame;

	bool simulate;
	bool first_mouse;

	float last_mouse_x;
	float last_mouse_y;
	short forward;
	short right;
	short up;

	vec3 position;
	vec3 rotation;
	mat4 model_matrix;
	mat4 view_matrix;
	mat4 projection_matrix;
};