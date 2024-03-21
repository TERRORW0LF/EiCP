#pragma once

#include "config.h"
#include "linear_algebra.h"

class Camera
{
public:
    Camera(vec3 position = vec3{0.0f, 0.0f, 0.0f}, float movement_speed = 3.0f, float sensitivity = 0.03f, float fov = 103.0f);

    mat4 get_view();
    void update_movement(short forward, short right, short up, double delta);
    void update_angle(float xoffset, float yoffset);

    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
    vec3 global_up;
    float yaw;
    float pitch;
    float movement_speed;
    float sensitivity;
    float fov;

private:
    void update_vectors();
};