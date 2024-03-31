#include "physics_engine.h"
#include <time.h>
#include <unordered_set>
#include <cassert>

/**
 * @brief Construct a new Physics Engine:: Physics Engine object
 *
 * @param cloth Pointer to cloth which is to be simulated
 * @param _gravity Gravitational force to be simulated
 * @param _mount Determines which points of the cloth are fixed in place
 */
PhysicsEngine::PhysicsEngine(ClothMesh *cloth, float3 _gravity, MountingType _mount) : cloth(cloth)
{
    gravity = _gravity;
    mount = _mount;
    velocity = std::vector<float3>(cloth->get_vertex_positions().size(), {0.f, 0.f, 0.f});
    old_position = std::vector<float3>(cloth->get_vertex_positions().size(), {0.0f, 0.0f, 0.0f});
    substeps = 20;
    delta_time = 1.0f;
}

/**
 * @brief Facade function to update the physics engine
 *
 */
void PhysicsEngine::update()
{
    auto current_time = std::chrono::high_resolution_clock::now();

    // Do not simulate first active simulation frame, since the difference would be massive.
    if (std::chrono::duration_cast<std::chrono::microseconds>(last_update.time_since_epoch()).count() == 0)
    {
        last_update = current_time;
        return;
    }

    // Determine the time step since last update.
    delta_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_update).count() / 1000000.0f;
    last_update = current_time;

    std::vector<float3> vertex_positions = cloth->get_vertex_positions();
    float spacing = cloth->get_rest_distance()[0];

    for (int i = 0; i < substeps; i++)
    {
        // Create hash map for efficient self collision checking. Each hash map cell has
        // one point in the default cloth state.
        SpatialHashStructure structure(vertex_positions, spacing, 20 * vertex_positions.size());
        update_step(vertex_positions, structure);
    }
    cloth->set_vertex_positions(vertex_positions);
}

/**
 * @brief Internal logic to update the physics engine
 * In this function, the physics engine is updated by a single step. This function is called by the update function.
 * Here, the physics engine updates the position of the cloth vertices based on the velocity and gravity.
 * Afterwards, the physics engine applies constraints to the cloth vertices to simulate the cloth's behavior.
 */
void PhysicsEngine::update_step(std::vector<float3> &vertex_positions, const SpatialHashStructure &structure)
{
    // Determine simulation time for this substep.
    float step_time = delta_time / substeps;

    // Simulation Position Update
    // For each particle in our system, determine our new velocity
    // and update the position accordingly.
    for (int i = 0; i < vertex_positions.size(); i++)
    {
        // reduce velocity by resistance to guarantee a steady state.
        // Also acts as air resistance.
        velocity[i] *= 0.999f;

        // add gravity to velocity
        velocity[i] += gravity * step_time;

        // save old position
        old_position[i] = vertex_positions[i];

        // update vertex position
        vertex_positions[i] += velocity[i] * step_time;
    }

    // Simulation Constraints

    // Constraint: Distance constraint
    // The distance constraint is a simple spring force between each pair of connected vertices.
    // It allows the cloth to stretch and compress, but not to bend.
    std::vector<float> rest_distance = cloth->get_rest_distance();
    float mass = 0.1f;
    float weight = 1 / mass;

    const auto &springs = cloth->get_unique_springs_ref();
    for (int i = 0; i < springs.size(); i++)
    {
        const auto edge = springs[i];
        unsigned int v1 = edge.data[0];
        unsigned int v2 = edge.data[1];

        float3 x1 = vertex_positions[v1];
        float3 x2 = vertex_positions[v2];

        float3 delta_x1 = x2 - x1;
        float length = delta_x1.length();
        delta_x1 /= length;
        delta_x1 *= (length - rest_distance[i]);

        float3 delta_x2 = delta_x1;

        delta_x1 *= weight / (weight + weight);
        delta_x2 *= -weight / (weight + weight);

        vertex_positions[v1] += delta_x1;
        vertex_positions[v2] += delta_x2;
    }

    float particle_radius = rest_distance[0] / 3.f;
    // particle_radius = 0.05f;
    assert(particle_radius <= rest_distance[0] / 2.f);
    // Constraint: Self collission
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

    if (mount == MountingType::CORNER_VERTEX)
    {
        unsigned int last = vertex_positions.size() - 1;
        last = old_position.size() - 1;
        vertex_positions[last] = old_position[last];
    }
    else if (mount == MountingType::MIDDLE_VERTEX)
    {
        int num_vertices_per_row = std::sqrt(vertex_positions.size() + 1);
        unsigned int some = num_vertices_per_row / 2 + num_vertices_per_row * num_vertices_per_row / 2;
        vertex_positions[some] = old_position[some];
    }
    else if (mount == MountingType::TOP_ROW)
    {
        int num_vertices_per_row = std::sqrt(vertex_positions.size() + 1);
        for (int i = num_vertices_per_row - 1; i < vertex_positions.size(); i += num_vertices_per_row)
        {
            vertex_positions[i] = old_position[i];
        }
    }
    else
    {
        std::exit(999);
    }

    // Update the velocity of each vertex by comparing the new position with the old position.
    for (int i = 0; i < vertex_positions.size(); i++)
    {
        velocity[i] = (vertex_positions[i] - old_position[i]) / step_time;
    }
}

ConcurrentPhysicsEngine::ConcurrentPhysicsEngine(ClothMesh *cloth, float3 gravity, MountingType m)
    : internal_engine(cloth, gravity, m)
{
    is_physics_computed.store(true);
    auto worker_f = [&]()
    {
        while (true)
        {
            is_physics_computed.wait(true); // blocks until is_physics_computed turns to false

            internal_engine.update();

            is_physics_computed.store(true);
            is_physics_computed.notify_one();
        }
    };

    worker = std::jthread(worker_f);
}

void ConcurrentPhysicsEngine::update()
{
    is_physics_computed.store(false);
    is_physics_computed.notify_one();
}

void ConcurrentPhysicsEngine::wait()
{
    is_physics_computed.wait(false); // blocks until is_physics_computed turns to true
}
