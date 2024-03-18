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