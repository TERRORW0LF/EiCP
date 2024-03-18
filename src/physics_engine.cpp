#include "physics_engine.h"

PhysicsEngine::PhysicsEngine(std::shared_ptr<ClothMesh> cloth, float3 gravity)
{
    this->cloth = cloth;
    this->gravity = gravity;
    this->velocity = std::vector<float3>(cloth->get_vertex_positions().size(), {0.1f, 0.000f, 0.000f});
    this->old_position = std::vector<float3>(cloth->get_vertex_positions().size(), {0.0f, 0.0f, 0.0f});
    this->substeps = 1;
    this->delta_time = 1.0f;
}

void PhysicsEngine::update()
{
    //this->delta_time = delta_time;
    for (int i = 0; i < substeps; i++) {
        //std::cout << "updating step " << velocity[0].x << std::endl;
        std::cout << "updating step " << old_position[0].x << std::endl;
        old_position[0].x = 5.0f;
        std::cout << "updating step " << old_position[0].x << std::endl;

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
        vertex_positions[vertex_counter].x += velocity[vertex_counter].x * time_counter;
        vertex_positions[vertex_counter].y += velocity[vertex_counter].y * time_counter;
        vertex_positions[vertex_counter].z += velocity[vertex_counter].z * time_counter;

        //update velocity
        velocity[vertex_counter].x += gravity.x * time_counter;
        velocity[vertex_counter].y += gravity.y * time_counter;
        velocity[vertex_counter].z += gravity.z * time_counter;

    }
    cloth->set_vertex_positions(vertex_positions);

    //update velocity
    //std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {
        velocity[vertex_counter].x = (vertex_positions[vertex_counter].x - old_position[vertex_counter].x) / time_counter;
        velocity[vertex_counter].y = (vertex_positions[vertex_counter].y - old_position[vertex_counter].y) / time_counter;
        velocity[vertex_counter].z = (vertex_positions[vertex_counter].z - old_position[vertex_counter].z) / time_counter;
        }


}