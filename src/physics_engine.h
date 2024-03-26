#pragma once

#include "cloth_mesh.h"
#include <vector>
#include <memory>
#include "algebraic_types.h"
#include "spatial_hash_structure.h"

class PhysicsEngine
{
public:
    PhysicsEngine(ClothMesh *cloth, float3 gravity);
    void update();

private:
    ClothMesh *cloth;
    float3 gravity;
    // float3 wind = {0.0f, 0.0f, 0.0f}; // for future use
    std::vector<float3> velocity;
    std::vector<float3> old_position;
    int substeps;
    float delta_time;
    void update_step(std::vector<float3>& vertex_positions, const SpatialHashStructure& structure);

    clock_t last_update = 0;

public:
};