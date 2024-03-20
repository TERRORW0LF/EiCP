#pragma once

#include "config.h"
#include "cloth_mesh.h"
#include "physics_engine.h"
#include "shader.h"
#include "linear_algebra.h"

class XPBDWindow
{
public:
	XPBDWindow();
	~XPBDWindow();

private:
	GLFWwindow *window;

	void handle_input(GLFWwindow *window, int key, int scancode, int action, int mods);

	void initialize_members();

	void print_help();

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

	bool simulate;
	bool view_update;
	float camera_speed;

	vec3 position;
	vec3 rotation;
	mat4 model_matrix;

	vec3 camera_pos;
	vec3 camera_movement;
	vec3 camera_front;
	vec3 camera_up;
	vec3 camera_right;
	mat4 view_matrix;
	mat4 projection_matrix;
};