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
mat4 view(vec3 pos, vec3 forward, vec3 global_up)
{
    // Normalize the vectors to guarantee that the transposed
    // matrix is the inverse.
    forward = normalize(forward);
    vec3 right = normalize(forward % global_up);
    vec3 up = normalize(right % forward);

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

    matrix.entries[12] = -(right * pos);
    matrix.entries[13] = -(up * pos);
    matrix.entries[14] = forward * pos;
    matrix.entries[15] = 1.0f;

    return matrix;
}

mat4 projection(float fov, float aspect_ratio, float near, float far)
{
    // Convert to radians and half it since the visible range is [-theta, theta].
    fov = fov * PI / 360;
    float tan = tanf(fov);

    // Flip sign bc things are only visible in the negative z direction.
    near = -near;
    far = -far;

    // Fill matrix with 0s.
    mat4 matrix;
    for (int i = 0; i < 16; i++)
    {
        matrix.entries[i] = 0.0f;
    }

    matrix.entries[0] = 1.0f / (aspect_ratio * tan);
    matrix.entries[5] = 1.0f / tan;
    matrix.entries[10] = -(near + far) / (near - far);
    matrix.entries[11] = -1.0f;
    matrix.entries[14] = 2 * near * far / (near - far);

    return matrix;
}

/**
 * @param v The input vector
 * @returns The length of the vector
 *
 * @brief Calculates the length of a vector.
 */
float length(vec3 v)
{
    return std::sqrt(v * v);
}

/**
 * @param v The vector to normalize.
 * @returns The normalized input vector.
 * @brief Normalizes a vector. The resulting length is 1.
 */
vec3 normalize(vec3 v)
{
    vec3 w;

    float length_v = length(v);

    // If v = 0 then return v to avoid returning nan.
    if (length_v == 0.0f)
    {
        return v;
    }

    w.entries[0] = v.entries[0] / length_v;
    w.entries[1] = v.entries[1] / length_v;
    w.entries[2] = v.entries[2] / length_v;

    return w;
}