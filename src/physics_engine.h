#pragma once

#include "primitives.h"
#include "cloth_mesh.h"
#include <vector>
#include <memory>



class PhysicsEngine
{
    public:
        PhysicsEngine(std::shared_ptr<ClothMesh> cloth, float3 gravity);
        void update();
        //~PhysicsEngine();

    private:
        std::shared_ptr<ClothMesh> cloth;
        float3 gravity;
        //float3 wind = {0.0f, 0.0f, 0.0f};
        std::vector<float3> velocity;
        std::vector<float3> old_position;
        int substeps;
        float delta_time;
        void update_step();

    public:


};