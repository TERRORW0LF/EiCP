#include "physics_engine.h"
#include <time.h>


PhysicsEngine::PhysicsEngine(ClothMesh* cloth, float3 gravity) :
    cloth(cloth)
{
    gravity = gravity;
    velocity = std::vector<float3>(cloth->get_vertex_positions().size(), { 0.1f, 0.000f, 0.000f });
    old_position = std::vector<float3>(cloth->get_vertex_positions().size(), { 0.0f, 0.0f, 0.0f });
    substeps = 1;
    delta_time = 1.0f;
}

void PhysicsEngine::update()
{
    clock_t current_time = clock();
    if (last_update == 0) {
        last_update = current_time;
        return;
    }

    delta_time = current_time - last_update;
    last_update = current_time;

    for (int i = 0; i < substeps; i++) {
        //std::cout << "updating step " << velocity[0].x << std::endl;
        //std::cout << "updating step " << old_position[0].data[0] << std::endl;
        //old_position[0].data[0] = 5.0f;
        //std::cout << "updating step " << old_position[0].data[0] << std::endl;


        update_step();
    }

}

void PhysicsEngine::update_step()
{
    float time_counter = ((float)delta_time) / substeps;
    std::vector<float3> vertex_positions = cloth->get_vertex_positions();//TODO move this out of the ministep
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {

        //update velocity
        velocity[vertex_counter] += gravity * time_counter;

        //save old position
        old_position[vertex_counter] = vertex_positions[vertex_counter];

        //update vertex position
        vertex_positions[vertex_counter] += velocity[vertex_counter] * time_counter;


    }
    
    //solve the constraints

    //Constraint 1: top left and top right must stay in place




    //update velocity
    //std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {
        velocity[vertex_counter] = (vertex_positions[vertex_counter] - old_position[vertex_counter]) / time_counter;
    }

    cloth->set_vertex_positions(vertex_positions);

}