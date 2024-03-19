#pragma once
#include <cmath>

#define PI 3.1415962

struct mat4
{
    float entries[16];
};

struct vec3
{
    float entries[3];
};

mat4 transform(vec3 translation);
mat4 rotate(vec3 rotation);
mat4 scale(float scale);
mat4 model(vec3 translation, vec3 rotation, float scale);
mat4 view(vec3 from, vec3 to);
mat4 projection(float fov, float aspect_ratio, float near, float far);

float dot(vec3 u, vec3 v);
vec3 normalize(vec3 v);
vec3 cross(vec3 u, vec3 v);