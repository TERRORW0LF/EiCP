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

	std::unique_ptr<vec3> position;
	std::unique_ptr<vec3> rotation;
	std::unique_ptr<mat4> model_matrix;

	std::unique_ptr<vec3> camera_pos;
	std::unique_ptr<vec3> camera_movement;
	std::unique_ptr<vec3> camera_front;
	std::unique_ptr<vec3> camera_up;
	std::unique_ptr<vec3> camera_right;
	std::unique_ptr<mat4> view_matrix;
	std::unique_ptr<mat4> projection_matrix;
};