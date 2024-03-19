#pragma once
#include "config.h"
#include "cloth_mesh.h"
#include "physics_engine.h"
#include "shader.h"
#include "linear_algebra.h"

class XPBDWindow {
public:
	XPBDWindow();
	~XPBDWindow();

private:
	GLFWwindow* window;


	void handle_input(GLFWwindow* window, int key, int scancode, int action, int mods);

	void initialize_members();

public:
	void update_window();
	//blocking!
	void enter_update_loop();

	//current members:
private:
	std::unique_ptr<ClothMesh> cloth;
	std::unique_ptr<PhysicsEngine> clothPhysics;
	std::unique_ptr<Shader> shader;

	bool simulate;

	vec3 cloth_position;
	vec3 cloth_rotation;
	mat4 model_matrix;

	vec3 camera_pos;
	vec3 camera_target;
	mat4 view_matrix;
};