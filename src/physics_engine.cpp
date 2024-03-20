#include "physics_engine.h"
#include <time.h>
#include <unordered_set>
#include "spatial_hash_structure.h"

/**
 * @brief Construct a new Physics Engine:: Physics Engine object
 *
 * @param cloth Pointer to cloth which is to be simulated
 * @param _gravity Gravitational force to be simulated
 */
PhysicsEngine::PhysicsEngine(ClothMesh *cloth, float3 _gravity) : cloth(cloth)
{
    gravity = _gravity;
    velocity = std::vector<float3>(cloth->get_vertex_positions().size(), {0.f, 0.f, 0.f});
    old_position = std::vector<float3>(cloth->get_vertex_positions().size(), {0.0f, 0.0f, 0.0f});
    substeps = 10;
    delta_time = 1.0f;
}

/**
 * @brief Facade function to update the physics engine
 *
 */
void PhysicsEngine::update()
{
    clock_t current_time = clock();
    if (last_update == 0)
    {
        last_update = current_time;
        return;
    }

    delta_time = current_time - last_update;
    delta_time = 10.f;
    last_update = current_time;

    for (int i = 0; i < substeps; i++)
    {
        update_step();
    }
}

/**
 * @brief Internal logic to update the physics engine
 * In this function, the physics engine is updated by a single step. This function is called by the update function.
 * Here, the physics engine updates the position of the cloth vertices based on the velocity and gravity. Afterwards, the physics engine applies constraints to the cloth vertices to simulate the cloth's behavior.
 */
void PhysicsEngine::update_step()
{
    float time_counter = ((float)delta_time) / substeps;
    std::vector<float3> vertex_positions = cloth->get_vertex_positions(); // TODO move this out of the ministep

    SpatialHashStructure structure(vertex_positions, 0.01f, 2 * vertex_positions.size());

    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++)
    {

        //reduce velocity by resistance
        velocity[vertex_counter] *= 0.999f;

        // add gravity to velocity
        velocity[vertex_counter] += gravity * time_counter;

        // save old position
        old_position[vertex_counter] = vertex_positions[vertex_counter];

        // update vertex position
        vertex_positions[vertex_counter] += velocity[vertex_counter] * time_counter;
    }

    // Simulation Constraints

    // Constraint: Distance constraint
    // The distance constraint is a simple spring force between each pair of connected vertices. It allows the cloth to stretch and compress, but not to bend.

    // iterate over triangles
    // store processed edges in a set
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
    struct KeyHasher
    {
        std::size_t operator()(const RealVector<unsigned int, 2> &k) const
        {
            return ((std::hash<unsigned int>()(k.data[0]) ^ (std::hash<unsigned int>()(k.data[1]) << 1)) >> 1);
        }
    };

    std::unordered_set<RealVector<unsigned int, 2>, KeyHasher> processed_edges;

    float rest_distance = 0.15f;
    float mass = 0.1f;
    float weight = 1 / mass;

    for (const uint3 &triangle : cloth->get_triangles_ref())
    {
        for (int i = 0; i < 3; i++)
        {
            unsigned int v1 = triangle.data[i];
            unsigned int v2 = triangle.data[(i + 1) % 3];
            RealVector<unsigned int, 2> edge_index;
            edge_index.data[0] = v1;
            edge_index.data[1] = v2;
            if (processed_edges.contains(edge_index))
                continue;

            float3 x1 = vertex_positions[v1];
            float3 x2 = vertex_positions[v2];

            float3 delta_x1 = x2 - x1;
            float length = delta_x1.length();
            delta_x1 /= length;
            delta_x1 *= (length - rest_distance);

            float3 delta_x2 = delta_x1;

            delta_x1 *= weight / (weight + weight);
            delta_x2 *= -weight / (weight + weight);

            vertex_positions[v1] += delta_x1;
            vertex_positions[v2] += delta_x2;

            processed_edges.insert(edge_index);
        }
    }

    // Constraint: Self collission
    float particle_radius = 0.05f;
    // iterate over vertices
    // iterate over neighboring cells
    // iterate over vertices in that cell
    // if they are too close to each other -> push them apart
    for (int v_i = 0; v_i < vertex_positions.size(); v_i++)
    {
        auto &v_p = vertex_positions[v_i];
        auto neighbor_cells = structure.compute_neighbor_cells(v_p);
        for (int n : neighbor_cells)
        {
            auto [first, last] = structure.get_particle_range_in_cell(n);
            for (auto neighbor_particle = first; neighbor_particle < last; neighbor_particle++)
            {
                auto neighbor_particle_index = structure.get_particles_arr()[neighbor_particle];
                auto &neighbor_particle_position = vertex_positions[neighbor_particle_index];

                auto v_p_minus_neighbor_particle_position = v_p - neighbor_particle_position;
                auto v_p_minus_neighbor_particle_position_length = v_p_minus_neighbor_particle_position.length();
                if (v_p_minus_neighbor_particle_position_length > 2 * particle_radius)
                    continue;
                if (v_i == neighbor_particle_index)
                    continue;

                // particles are too close!
                // do some computation to push them apart!

                // normalize
                v_p_minus_neighbor_particle_position /= v_p_minus_neighbor_particle_position_length;

                float length_to_push_apart = 2 * particle_radius - v_p_minus_neighbor_particle_position_length;

                v_p += (v_p_minus_neighbor_particle_position * (0.5 * length_to_push_apart));
                neighbor_particle_position -= (v_p_minus_neighbor_particle_position * (0.5 * length_to_push_apart));
            }
        }
    }

    // Constraint: top left and top right must stay in place
    // This constraint is a simple position constraint that keeps the top left and top right vertices in place. Hence, the cloth will not fall down and we can see the effect of the other constraints.
    unsigned int last = vertex_positions.size() - 1;
    last = 0;
    vertex_positions[last] = old_position[last];

    // Update the velocity of each vertex by comparing the new position with the old position.
    for (int vertex_counter = 0; vertex_counter < vertex_positions.size(); vertex_counter++)
    {
        velocity[vertex_counter] = (vertex_positions[vertex_counter] - old_position[vertex_counter]) / time_counter;
    }

    cloth->set_vertex_positions(vertex_positions);
}