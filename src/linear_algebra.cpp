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
    // Calculate the angles in radian.
    float alpha = rotation.entries[0] * PI / 180.0f;
    float beta = rotation.entries[1] * PI / 180.0f;
    float gamma = rotation.entries[2] * PI / 180.0f;

    // Precompute cos and sin of angles.
    float cosa = cosf(alpha);
    float sina = sinf(alpha);
    float cosb = cosf(beta);
    float sinb = sinf(beta);
    float cosc = cosf(gamma);
    float sinc = sinf(gamma);

    // Combined rotation, scaling, and translation matrix.
    // The matrix multiplication of the three is hard coded here.
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
    // Initialize matrix with 0s.
    mat4 matrix;
    for (int i = 0; i < 16; i++)
    {
        matrix.entries[i] = 0.0f;
    }

    // Set indentity matrix to keep base position.
    matrix.entries[0] = 1.0f;
    matrix.entries[5] = 1.0f;
    matrix.entries[10] = 1.0f;
    matrix.entries[15] = 1.0f;

    // Shift base position by translation vector.
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
    // Calculate the angles in radian.
    float alpha = rotate.entries[0] * PI / 180.0f;
    float beta = rotate.entries[1] * PI / 180.0f;
    float gamma = rotate.entries[2] * PI / 180.0f;

    // Precompute cos and sin of angles.
    float cosa = cosf(alpha);
    float sina = sinf(alpha);
    float cosb = cosf(beta);
    float sinb = sinf(beta);
    float cosc = cosf(gamma);
    float sinc = sinf(gamma);

    // Combined 3D rotation matrix. Copied from
    // https://en.wikipedia.org/wiki/Rotation_matrix
    // And adjusted for a combined rotation and translation matrix.
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

/**
 * Creates a translation matrix from world to camera view.
 * The values of [from] and [to] must be in [-1.0f, 1.0f].
 *
 * @param from The current camera position.
 * @param to A point on the view line of the camera.
 * @returns A translation matrix from world to camera coordinates.
 * @brief Creates a view transformation matrix from world to camera view.
 */
mat4 view(vec3 from, vec3 to)
{
    // Helper for finding the camera right vector.
    vec3 global_up = {0.0f, 0.0f, 1.0f};

    // Normalize the vectors to guarantee that the transposed
    // matrix is the inverse.
    // The viewing direction of the camera.
    vec3 forward = {
        to.entries[0] - from.entries[0],
        to.entries[1] - from.entries[1],
        to.entries[2] - from.entries[2]};
    forward = normalize(forward);

    // The correct cross product can be found using the right hand rule.

    // The right facing vector of the camera.
    // Normal of the plane of the forward and up vector.
    vec3 right = normalize(cross(forward, global_up));

    // Set camera up vector.
    // Normal of the camera right and forward vector.
    vec3 up = normalize(cross(right, forward));

    // Inverse of the world to camera translation matrix.
    // The world to camera translation is the matrix consisting of
    // the camera base vectors (up, right, forwards) as its columns.
    // This is true bc the world vector space is the base vector space.
    // Since the camera vectors are orthononormal the inverse is the
    // transposed matrix.
    // Since in OpenGL the forward vector points behind the camera,
    // we need to negate it here.
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

/**
 * @param u The first input vector.
 * @param v The second input vector.
 * @returns The dot prodcut of the input vectors.
 * @brief Calculates the dot product.
 */
float dot(vec3 u, vec3 v)
{
    return u.entries[0] * v.entries[0] + u.entries[1] * v.entries[1] + u.entries[2] * v.entries[2];
}

/**
 * @param v The vector to normalize.
 * @returns The normalized input vector.
 * @brief Normalizes a vector. The resulting length is 1.
 */
vec3 normalize(vec3 v)
{
    vec3 w;

    float length = sqrtf(dot(v, v));
    w.entries[0] = v.entries[0] / length;
    w.entries[1] = v.entries[1] / length;
    w.entries[2] = v.entries[2] / length;

    return w;
}

/**
 * Calculates the cross product of two vectors.
 * The cross product is the normal vector to the plane
 * created by the two input vectors.
 *
 * @param u The first input vector.
 * @param v The second input vector.
 * @returns The cross product of the input vectors.
 * @brief Calculates the cross product.
 */
vec3 cross(vec3 u, vec3 v)
{
    vec3 w;

    w.entries[0] = u.entries[1] * v.entries[2] - u.entries[2] * v.entries[1];
    w.entries[1] = -(u.entries[0] * v.entries[2] - u.entries[2] * v.entries[0]);
    w.entries[2] = u.entries[0] * v.entries[1] - u.entries[1] * v.entries[1];

    return w;
}