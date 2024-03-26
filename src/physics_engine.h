#pragma once

#include "cloth_mesh.h"
#include <vector>
#include <memory>
#include "algebraic_types.h"
#include "spatial_hash_structure.h"
#include <condition_variable>

enum MountingType { CORNER_VERTEX = GLFW_KEY_1, TOP_ROW = GLFW_KEY_2, MIDDLE_VERTEX = GLFW_KEY_3 };


class PhysicsEngine
{
public:
    PhysicsEngine(ClothMesh *cloth, float3 gravity, MountingType mount);
    void update();

private:
    ClothMesh *cloth;
    float3 gravity;
    MountingType mount;
    // float3 wind = {0.0f, 0.0f, 0.0f}; // for future use
    std::vector<float3> velocity;
    std::vector<float3> old_position;
    int substeps;
    float delta_time;
    void update_step(std::vector<float3>& vertex_positions, const SpatialHashStructure& structure);

    clock_t last_update = 0;

public:
};


class ConcurrentPhysicsEngine {
public:
    ConcurrentPhysicsEngine(ClothMesh* cloth, float3 gravity, MountingType m);
    void update();
    void wait();

private:
    PhysicsEngine internal_engine;
    std::jthread worker;

    std::atomic<bool> is_physics_computed;
};