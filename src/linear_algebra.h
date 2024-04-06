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

    // Modified code of this stackoverflow answer:
    // https://stackoverflow.com/a/14047520
    // Implement functions here so that the compiler can inline them.
    inline bool operator==(const vec3 &v) const
    {
        return entries[0] == v.entries[0] && entries[1] == v.entries[1] && entries[2] == v.entries[2];
    }
    inline vec3 operator-() const
    {
        return {-entries[0], -entries[1], -entries[2]};
    }
    inline vec3 operator+(const vec3 &v) const
    {
        return {entries[0] + v.entries[0], entries[1] + v.entries[1], entries[2] + v.entries[2]};
    }
    inline vec3 &operator+=(const vec3 &v)
    {
        *this = *this + v;
        return *this;
    }
    inline vec3 operator-(const vec3 &v) const
    {
        return *this + -v;
    }
    inline vec3 &operator-=(const vec3 &v)
    {
        *this = *this - v;
        return *this;
    }
    inline friend vec3 operator*(const float s, const vec3 &v)
    {
        return {s * v.entries[0], s * v.entries[1], s * v.entries[2]};
    }
    inline friend vec3 &operator*=(const float s, vec3 &v)
    {
        v = s * v;
        return v;
    }
    inline friend vec3 operator*(const vec3 &v, const float s)
    {
        return s * v;
    }
    inline friend vec3 &operator*=(vec3 &v, const float s)
    {
        return s *= v;
    }
    inline friend vec3 operator/(const float s, const vec3 &v)
    {
        return {v.entries[0] / s, v.entries[1] / s, v.entries[2] / s};
    }
    inline friend vec3 operator/(const vec3 &v, const float s)
    {
        return s / v;
    }
    inline friend vec3 &operator/=(vec3 &v, const float s)
    {
        v = v / s;
        return v;
    }
    inline float operator*(const vec3 &v) const
    {
        return entries[0] * v.entries[0] + entries[1] * v.entries[1] + entries[2] * v.entries[2];
    }
    inline vec3 operator%(const vec3 &v) const
    {
        return {(entries[1] * v.entries[2]) - (entries[2] * v.entries[1]),
                (entries[2] * v.entries[0]) - (entries[0] * v.entries[2]),
                (entries[0] * v.entries[1]) - (entries[1] * v.entries[0])};
    }
    inline vec3 &operator%=(const vec3 &v)
    {
        *this = *this % v;
        return *this;
    }
};

mat4 transform(vec3 translation);
mat4 rotate(vec3 rotation);
mat4 scale(float scale);
mat4 model(vec3 translation, vec3 rotation, float scale);
mat4 view(vec3 pos, vec3 forward, vec3 global_up);
mat4 projection(float fov, float aspect_ratio, float near, float far);

float length(vec3 v);
vec3 normalize(vec3 v);