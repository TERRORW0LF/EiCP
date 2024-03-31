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
    float spacing = cloth->get_rest_distance_ref()[0];

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
    std::vector<float> rest_distance = cloth->get_rest_distance_ref();
    std::vector<float> mass = cloth->get_mass_ref();

    const auto &springs = cloth->get_unique_springs_ref();
    for (int i = 0; i < springs.size(); i++)
    {
        // Get vertices of the edge.
        const auto edge = springs[i];
        unsigned int v1 = edge.data[0];
        unsigned int v2 = edge.data[1];

        float3 x1 = vertex_positions[v1];
        float3 x2 = vertex_positions[v2];
        float mass1 = mass[v1];
        float mass2 = mass[v2];

        // Determine direction vector of spring and
        // set its length to the offset from the rest distance.
        float3 delta = x2 - x1;
        float length = delta.length();
        delta /= length;
        delta *= (length - rest_distance[i]);

        float3 delta_x1 = delta;
        float3 delta_x2 = delta * -1;

        // Distribute the offset to both vertices based on their weight.
        delta_x1 *= mass2 / (mass1 + mass2);
        delta_x2 *= mass1 / (mass1 + mass2);

        // Constraint: top left and top right must stay in place
        // This constraint is a simple position constraint
        // that keeps the top left and top right vertices in place.
        // Hence, the cloth will not fall down and we can see the
        // effect of the other constraints.
        if (mount == MountingType::CORNER_VERTEX)
        {
            unsigned int last = vertex_positions.size() - 1;
            std::cout << v1 << " " << v2 << " " << last << std::endl;
            if (v1 == last)
            {
                delta_x1 *= 0;
                delta_x2 = delta * -1;
            }
            else if (v2 == last)
            {
                delta_x1 = delta;
                delta_x2 *= 0;
            }
        }
        else if (mount == MountingType::MIDDLE_VERTEX)
        {
            int num_cols = std::sqrt(vertex_positions.size() + 1);
            unsigned int middle = num_cols / 2 + num_cols * num_cols / 2;
            if (v1 == middle)
            {
                delta_x1 *= 0;
                delta_x2 = delta * -1;
            }
            else if (v2 == middle)
            {
                delta_x1 = delta;
                delta_x2 *= 0;
            }
        }
        else if (mount == MountingType::TOP_ROW)
        {
            int num_cols = std::sqrt(vertex_positions.size() + 1);
            int low = vertex_positions.size() - 1 - num_cols;

            if (v1 >= low)
            {
                delta_x1 *= 0;
                delta_x2 = delta * -1;
            }
            if (v2 >= low)
            {
                delta_x1 = delta;
                delta_x2 *= 0;
            }
            if (v2 >= low && v1 >= low)
            {
                delta_x1 = delta;
                delta_x2 *= 0;
            }
        }

        vertex_positions[v1] += delta_x1;
        vertex_positions[v2] += delta_x2;
    }

    // Constraint: Self collission
    // iterate over vertices
    // iterate over neighboring cells
    // iterate over vertices in that cell
    // if they are too close to each other -> push them apart
    float particle_radius = rest_distance[0] / 3.f;

    for (int i = 0; i < vertex_positions.size(); i++)
    {
        auto &vertex_pos = vertex_positions[i];
        auto neighbor_cells = structure.compute_neighbor_cells(vertex_pos);
        for (int neighbor_cell : neighbor_cells)
        {
            auto [first, last] = structure.get_particle_range_in_cell(neighbor_cell);
            for (auto j = first; j < last; j++)
            {
                auto particle_index = structure.get_particles_arr()[j];
                auto &particle_position = vertex_positions[particle_index];

                auto local_particle_pos = vertex_pos - particle_position;
                auto local_length = local_particle_pos.length();
                if (local_length > 2 * particle_radius)
                    continue;
                if (i == particle_index)
                    continue;

                // particles are too close!
                // do some computation to push them apart!

                // normalize
                local_particle_pos /= local_length;

                float adjustment = 2 * particle_radius - local_length;

                vertex_pos += (local_particle_pos * (0.5 * adjustment));
                particle_position -= (local_particle_pos * (0.5 * adjustment));
            }
        }
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
