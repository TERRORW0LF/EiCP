#include "linear_algebra.h"

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
    matrix.entries[5] = sina * sinb * sinc - cosa * cosc;
    matrix.entries[6] = sina * cosb;
    matrix.entries[7] = 0.0f;

    matrix.entries[8] = cosa * sinb * cosc + sina * sinc;
    matrix.entries[9] = cosa * sinb * sinc - sina * cosc;
    matrix.entries[10] = cosa * cosb;
    matrix.entries[11] = 0.0;

    matrix.entries[12] = 0.0;
    matrix.entries[13] = 0.0;
    matrix.entries[14] = 0.0;
    matrix.entries[15] = 1.0;

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