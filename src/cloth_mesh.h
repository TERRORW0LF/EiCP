#pragma once
#include "config.h"
#include "algebraic_types.h"
#include "linear_algebra.h"
#include "obj_reader.h"

class ClothMesh
{
public:
    ClothMesh(const std::string &cloth_path, vec3 color);
    void draw();
    ~ClothMesh();

private:
    // EBO: buffer to triangle indices,
    // VAO: buffer to geometry and topology of this mesh (?)
    // element_count: number of faces/triangles
    // VBOs[0]: buffer to vertex positions
    // VBOs[1]: buffer to vertex colors
    // VBOs[2]: buffer to vertex normals
    unsigned int EBO, VAO, element_count;
    std::vector<unsigned int> VBOs;

    mutable bool vertex_positions_invalid = true;
    std::vector<float3> vertex_positions;
    std::vector<uint3> triangles;

    void compute_and_store_normals();
    void compute_normals(std::vector<float3> &out);

    // Subset of unique edges, containing only straight edges
    // meaning that diagonal edges have been removed
    std::vector<RealVector<unsigned int, 2>> unique_springs;

    // The rest distance between two nodes
    // computed as the average edge length
    std::vector<float> rest_distance;

    // The mass of the particles.
    std::vector<float> mass;

public:
    const std::vector<float> &get_rest_distance_ref() const;
    const std::vector<float> &get_mass_ref() const;

    std::vector<float3> get_vertex_positions() const;
    // this will invalidate the vertex positions array
    void set_vertex_positions(const std::vector<float3> &new_vertex_positions);

    // topology remains unchanged, so we dont need a setter!
    std::vector<uint3> get_triangles() const;
    const std::vector<uint3> &get_triangles_ref() const;
    const std::vector<RealVector<unsigned int, 2>> &get_unique_springs_ref() const;
};
