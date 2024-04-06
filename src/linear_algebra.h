#pragma once
#include <cmath>

// Approximation of pi.
#define PI 3.1415962

/**
 * @brief Holds a 4x4 matrix in column major order.
 */
struct mat4
{
    float entries[16];
};

/**
 * A float vector of length 3 supporting additon, subtraction,
 * scaling, equality, dot product, and cross product.
 * @brief Holds a vector of length 3.
 */
struct vec3
{
    float entries[3];

    bool operator==(const vec3 &v) const;
    vec3 operator-() const;
    vec3 operator+(const vec3 &v) const;
    vec3 &operator+=(const vec3 &v);
    vec3 operator-(const vec3 &v) const;
    vec3 &operator-=(const vec3 &v);
    // Scalar
    friend vec3 operator*(const float s, const vec3 &v);
    friend vec3 &operator*=(const float s, vec3 &v);
    friend vec3 operator*(const vec3 &v, const float s);
    friend vec3 &operator*=(vec3 &v, const float s);
    friend vec3 operator/(const float s, const vec3 &v);
    friend vec3 &operator/=(const float s, vec3 &v);
    friend vec3 operator/(const vec3 &v, const float s);
    friend vec3 &operator/=(vec3 &v, const float s);
    // Dot product
    float operator*(const vec3 &v) const;
    // Cross product
    vec3 operator%(const vec3 &v) const;
    vec3 &operator%=(const vec3 &v);
};

mat4 transform(vec3 translation);
mat4 rotate(vec3 rotation);
mat4 scale(float scale);
mat4 model(vec3 translation, vec3 rotation, float scale);
mat4 view(vec3 pos, vec3 forward, vec3 global_up);
mat4 projection(float fov, float aspect_ratio, float near, float far);

float length(vec3 v);
vec3 normalize(vec3 v);