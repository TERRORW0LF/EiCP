#include "physics_engine.h"

PhysicsEngine::PhysicsEngine(std::shared_ptr<ClothMesh> cloth, float3 gravity)
{
    this->cloth = cloth;
    this->gravity = gravity;
    this->velocity = std::vector<float3>(cloth->get_vertex_positions().size(), {0.001f, 0.000f, 0.000f});
    this->old_position = std::vector<float3>(cloth->get_vertex_positions().size(), {0.0f, 0.0f, 0.0f});
    this->substeps = 1;
    this->delta_time = 1.0f;
}

void PhysicsEngine::update()
{
    //this->delta_time = delta_time;
    for (int i = 0; i < substeps; i++) {
        update_step();
    }

}

void PhysicsEngine::update_step()
{
    float time_counter = delta_time / substeps;
    std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {

        //save old position
        old_position[vertex_counter] = vertex_positions[vertex_counter];

        //update vertex position
        vertex_positions[vertex_counter] += velocity[vertex_counter] * time_counter;

        //update velocity
        velocity[vertex_counter] += gravity * time_counter;

    }
    cloth->set_vertex_positions(vertex_positions);

    //update velocity
    //std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {
        velocity[vertex_counter] = (vertex_positions[vertex_counter] - old_position[vertex_counter]) / time_counter;
        }


}