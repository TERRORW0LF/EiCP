#include "camera.h"

Camera::Camera(vec3 position, float movement_speed, float sensitivity, float fov)
    : position(position), movement_speed(movement_speed), sensitivity(sensitivity), fov(fov)
{
    global_up = {0.0f, 1.0f, 0.0f};
    forward = {0.0f, 0.0f, -1.0f};
    yaw = -90.0f;
    pitch = 0.0f;

    update_vectors();
}

mat4 Camera::get_view()
{
    return view(position, forward, global_up);
}

void Camera::update_movement(short forward, short right, short up, double delta)
{
    vec3 direction = this->forward * forward + this->right * right + global_up * up;
    position += normalize(direction) * movement_speed * delta;

    update_vectors();
}

void Camera::update_angle(float xoffset, float yoffset)
{
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    update_vectors();
}

void Camera::update_vectors()
{
    // Camera angle
    const float yaw_rad = yaw * PI / 180;
    const float pitch_rad = pitch * PI / 180;
    vec3 direction = {
        cosf(yaw_rad) * cosf(pitch_rad),
        sinf(pitch_rad),
        sinf(yaw_rad) * cosf(pitch_rad)};
    forward = normalize(direction);
    right = normalize(forward % global_up);
    up = normalize(right % forward);
}