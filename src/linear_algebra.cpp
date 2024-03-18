#include "linear_algebra.h"

/**
 * Combines rotating, scaling, and translating into a single matrix.
 * The order of the translation vector is x, y, z.
 * The order of the rotation vector is alpha, beta, gamma corresponding
 * to z, y, and x rotation respectively.
 * For performance reasons the matrix multiplication is hard coded and
 * does not rely on the individual transformation functions.
 *
 * @param translation The vector to shift by.
 * @param rotation The vector holding the rotation angles.
 * @param scale The scale to apply.
 * @returns A combined rotation, scaling, and translation matrix.
 * @brief Creates a complete model matrix.
 */
mat4 model(vec3 translation, vec3 rotation, float scale)
{
    float alpha = rotation.entries[0] * PI / 180.0f;
    float beta = rotation.entries[1] * PI / 180.0f;
    float gamma = rotation.entries[2] * PI / 180.0f;

    float cosa = cosf(alpha);
    float sina = sinf(alpha);
    float cosb = cosf(beta);
    float sinb = sinf(beta);
    float cosc = cosf(gamma);
    float sinc = sinf(gamma);

    mat4 matrix;
    matrix.entries[0] = scale * cosb * cosc;
    matrix.entries[1] = cosb * sinc;
    matrix.entries[2] = -sinb;
    matrix.entries[3] = 0.0f;

    matrix.entries[4] = sina * sinb * cosc - cosa * sinc;
    matrix.entries[5] = scale * sina * sinb * sinc + cosa * cosc;
    matrix.entries[6] = sina * cosb;
    matrix.entries[7] = 0.0f;

    matrix.entries[8] = cosa * sinb * cosc + sina * sinc;
    matrix.entries[9] = cosa * sinb * sinc - sina * cosc;
    matrix.entries[10] = scale * cosa * cosb;
    matrix.entries[11] = 0.0f;

    matrix.entries[12] = translation.entries[0];
    matrix.entries[13] = translation.entries[1];
    matrix.entries[14] = translation.entries[2];
    matrix.entries[15] = 1.0f;

    return matrix;
}

/**
 * @param translation A [vec3] containing in order: x, y, and z
 * values.
 * @returns A four 4x4 transformation matrix.
 * @brief Creates a 4x4 fransformation matrix which translates a vector
 * by the given vector.
 */
mat4 transform(vec3 translation)
{
    mat4 matrix;
    for (int i = 0; i < 16; i++)
    {
        matrix.entries[i] = 0.0f;
    }

    matrix.entries[0] = 1.0f;
    matrix.entries[5] = 1.0f;
    matrix.entries[10] = 1.0f;
    matrix.entries[15] = 1.0f;

    matrix.entries[12] = translation.entries[0];
    matrix.entries[13] = translation.entries[1];
    matrix.entries[14] = translation.entries[2];

    return matrix;
}

/**
 * @param rotate A [vec3] containing in order: alpha, beta, and gamma
 * angles for z, y, and x rotation respectively in degrees.
 * @returns A four 4x4 rotation matrix.
 * @brief Creates a 4x4 rotation matrix based on the input vector.
 */
mat4 rotate(vec3 rotate)
{
    float alpha = rotate.entries[0] * PI / 180.0f;
    float beta = rotate.entries[1] * PI / 180.0f;
    float gamma = rotate.entries[2] * PI / 180.0f;

    float cosa = cosf(alpha);
    float sina = sinf(alpha);
    float cosb = cosf(beta);
    float sinb = sinf(beta);
    float cosc = cosf(gamma);
    float sinc = sinf(gamma);

    mat4 matrix;
    matrix.entries[0] = cosb * cosc;
    matrix.entries[1] = cosb * sinc;
    matrix.entries[2] = -sinb;
    matrix.entries[3] = 0.0f;

    matrix.entries[4] = sina * sinb * cosc - cosa * sinc;
    matrix.entries[5] = sina * sinb * sinc + cosa * cosc;
    matrix.entries[6] = sina * cosb;
    matrix.entries[7] = 0.0f;

    matrix.entries[8] = cosa * sinb * cosc + sina * sinc;
    matrix.entries[9] = cosa * sinb * sinc - sina * cosc;
    matrix.entries[10] = cosa * cosb;
    matrix.entries[11] = 0.0f;

    matrix.entries[12] = 0.0f;
    matrix.entries[13] = 0.0f;
    matrix.entries[14] = 0.0f;
    matrix.entries[15] = 1.0f;

    return matrix;
}

/**
 * @param scale The factor the matrix should scale by.
 * @returns A four 4x4 scaling matrix.
 * @brief Creates a 4x4 scaling matrix based on the input scale.
 */
mat4 scale(float scale)
{
    mat4 matrix;
    for (int i = 0; i < 16; i++)
    {
        matrix.entries[i] = 0.0f;
    }

    matrix.entries[0] = scale;
    matrix.entries[5] = scale;
    matrix.entries[10] = scale;
    matrix.entries[15] = 1.0f;

    return matrix;
}

mat4 view(vec3 from, vec3 to)
{
    vec3 global_up = {0.0f, 0.0f, 1.0f};
    vec3 forward = {
        to.entries[0] - from.entries[0],
        to.entries[1] - from.entries[1],
        to.entries[2] - from.entries[2]};
    forward = normalize(forward);
    vec3 right = normalize(cross(forward, global_up));
    vec3 up = normalize(cross(right, forward));

    mat4 matrix;
    matrix.entries[0] = right.entries[0];
    matrix.entries[1] = up.entries[0];
    matrix.entries[2] = -forward.entries[0];
    matrix.entries[3] = 0.0f;

    matrix.entries[4] = right.entries[1];
    matrix.entries[5] = up.entries[1];
    matrix.entries[6] = -forward.entries[1];
    matrix.entries[7] = 0.0f;

    matrix.entries[8] = right.entries[2];
    matrix.entries[9] = up.entries[2];
    matrix.entries[10] = -forward.entries[2];
    matrix.entries[11] = 0.0f;

    matrix.entries[12] = -dot(right, from);
    matrix.entries[13] = -dot(up, from);
    matrix.entries[14] = dot(forward, from);
    matrix.entries[15] = 1.0f;

    return matrix;
}

float dot(vec3 u, vec3 v)
{
    return u.entries[0] * v.entries[0] + u.entries[1] * v.entries[1] + u.entries[2] * v.entries[2];
}

vec3 normalize(vec3 v)
{
    vec3 w;

    float length = sqrtf(dot(v, v));
    w.entries[0] = v.entries[0] / length;
    w.entries[1] = v.entries[1] / length;
    w.entries[2] = v.entries[2] / length;

    return w;
}

vec3 cross(vec3 u, vec3 v)
{
    vec3 w;

    w.entries[0] = u.entries[1] * v.entries[2] - u.entries[2] * v.entries[1];
    w.entries[1] = -(u.entries[0] * v.entries[2] - u.entries[2] * v.entries[0]);
    w.entries[2] = u.entries[0] * v.entries[1] - u.entries[1] * v.entries[1];

    return w;
}