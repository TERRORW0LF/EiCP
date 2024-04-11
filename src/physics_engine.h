#pragma once

#include "cloth_mesh.h"
#include <vector>
#include <memory>
#include "algebraic_types.h"
#include "spatial_hash_structure.h"
#include <condition_variable>

enum MountingType
{
    CORNER_VERTEX = GLFW_KEY_1,
    TOP_ROW = GLFW_KEY_2,
    MIDDLE_VERTEX = GLFW_KEY_3,
    UNCONSTRAINED = GLFW_KEY_4
};

class PhysicsEngine
{
public:
    PhysicsEngine(ClothMesh *cloth, vec3 gravity, MountingType mount);
    void update();

private:
    ClothMesh *cloth;
    vec3 gravity;
    MountingType mount;
    std::vector<vec3> velocity;
    std::vector<vec3> old_position;
    int substeps;
    float delta_time;
    void update_step(std::vector<vec3> &vertex_positions, const SpatialHashStructure &structure);
    bool is_fixed(unsigned int size, unsigned int index) const;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_update;

public:
};

#ifndef __clang__
class ConcurrentPhysicsEngine
{
public:
    ConcurrentPhysicsEngine(ClothMesh *cloth, vec3 gravity, MountingType m);
    void update();
    void wait();

private:
    PhysicsEngine internal_engine;
    std::jthread worker;

    std::atomic<bool> is_physics_computed;
};
#endif