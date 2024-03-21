#include "camera.h"

/**
 * @param position The initial camera position.
 * @param movement_speed The speed of the camera in m/s.
 * @param sensitivity The mouse sensitivity.
 * @param fov The field of view of the camera.
 * @returns A camera object for moving through the scene.
 *
 * @brief The camera is used to move in the scene and view different angles.
 */
Camera::Camera(vec3 position, float movement_speed, float sensitivity, float fov)
    : position(position), movement_speed(movement_speed), sensitivity(sensitivity), fov(fov)
{
    global_up = {0.0f, 1.0f, 0.0f};
    forward = {0.0f, 0.0f, -1.0f};

    // We want to start out looking along the negative z axis.
    yaw = -90.0f;
    pitch = 0.0f;

    update_vectors();
}

/**
 * @returns The current view matrix for the camera position and facing angle.
 */
mat4 Camera::get_view()
{
    return view(position, forward, global_up);
}

/**
 * @param forward The scalar for the forward direction vector.
 * @param right The scalar for the right direction vector.
 * @param up The scalar for the up direction vector.
 * @param delta The current time step in seconds.
 *
 * @brief Updates the camera position based on the provided direction scalars.
 * The scalars are in the range of [-1, 1].
 */
void Camera::update_movement(short forward, short right, short up, double delta)
{
    vec3 direction = this->forward * forward + this->right * right + global_up * up;
    // Adjust for movement speed and current time step size.
    position += normalize(direction) * movement_speed * delta;
}

/**
 * @param xoffset The amount of pixels moved on the x axis.
 * @param yoffset The amount of pixels moved on the y axis.
 *
 * @brief Update the camera angle based on the mouse movement.
 */
void Camera::update_angle(float xoffset, float yoffset)
{
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    // Disallow rollover when looking up or down.
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    update_vectors();
}

/**
 * @brief Set the new camera base vectors based on the new yaw and pitch.
 */
void Camera::update_vectors()
{
    // Camera angle.
    const float yaw_rad = yaw * PI / 180;
    const float pitch_rad = pitch * PI / 180;

    // Update camera facing direction.
    // For yaw this is (cos(yaw), 1, sin(yaw))^T
    // For pitch this is (cos(pitch), sin(pitch), cos(pitch))^T
    vec3 direction = {
        cosf(yaw_rad) * cosf(pitch_rad),
        sinf(pitch_rad),
        sinf(yaw_rad) * cosf(pitch_rad)};

    // Set new camera base vectors.
    forward = normalize(direction);
    right = normalize(forward % global_up);
    up = normalize(right % forward);
}