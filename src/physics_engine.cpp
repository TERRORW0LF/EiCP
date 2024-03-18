#include "physics_engine.h"
#include <time.h>
#include <unordered_set>


PhysicsEngine::PhysicsEngine(ClothMesh* cloth, float3 _gravity) :
    cloth(cloth)
{
    gravity = _gravity;
    velocity = std::vector<float3>(cloth->get_vertex_positions().size(), { 0.f, 0.f, 0.f });
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
    //delta_time = 100.f;

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


    //Constraint 2: Distance constraint

    //iterate over triangles
    //store processed edges in a set

    //https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
    struct KeyHasher
    {
        std::size_t operator()(const RealVector<unsigned int, 2>& k) const
        {
            return ((std::hash<unsigned int>()(k.data[0])
                ^ (std::hash<unsigned int>()(k.data[1]) << 1)) >> 1);
        }
    };

    std::unordered_set<RealVector<unsigned int, 2>, KeyHasher> processed_edges;

    float rest_distance = 0.15f;
    float mass = 0.1f;
    float weight = 1 / mass;

    for (const uint3& triangle : cloth->get_triangles_ref()) {
        for (int i = 0; i < 3; i++) {
            unsigned int v1 = triangle.data[i];
            unsigned int v2 = triangle.data[(i + 1) % 3];
            RealVector<unsigned int, 2> edgeIndex;
            edgeIndex.data[0] = v1;
            edgeIndex.data[1] = v2;
            if (processed_edges.contains(edgeIndex))
                continue;

            float3 x1 = vertex_positions[v1];
            float3 x2 = vertex_positions[v2];

            float3 delta_x1 = x2 - x1;
            float length = delta_x1.length();
            delta_x1 /= length;
            delta_x1 *= (length - rest_distance);

            float3 delta_x2 = delta_x1;

            delta_x1 *= weight / (weight + weight);
            delta_x2 *= - weight / (weight + weight);

            vertex_positions[v1] += delta_x1;
            vertex_positions[v2] += delta_x2;

            processed_edges.insert(edgeIndex);
        }
    }

    //Constraint 1: top left and top right must stay in place
    unsigned int last = vertex_positions.size() - 1;
    vertex_positions[last] = old_position[last];


    //update velocity
    //std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++) {
        velocity[vertex_counter] = (vertex_positions[vertex_counter] - old_position[vertex_counter]) / time_counter;
    }

    cloth->set_vertex_positions(vertex_positions);

}