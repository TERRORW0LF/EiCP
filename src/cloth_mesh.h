#pragma once
#include "config.h"
#include "primitives.h"
#include "algebraic_types.h"

class ClothMesh
{
public:
    ClothMesh(const std::string& cloth_path, float color[3]);
    void draw();
    ~ClothMesh();

private:
    //EBO: buffer to triangle indices,
    //VAO: buffer to geometry and topology of this mesh (?)
    //element_count: number of faces/triangles
    //VBOs[0]: buffer to vertex positions
    //VBOs[1]: buffer to vertex colors
    //VBOs[2]: buffer to vertex normals
    unsigned int EBO, VAO, element_count;
    std::vector<unsigned int> VBOs;


    mutable bool vertex_positions_invalid = true;
    std::vector<float3> vertex_positions;
    std::vector<uint3> triangles;

    void compute_and_store_normals();
    void compute_normals(std::vector<float3>& out);
public:

    std::vector<float3> get_vertex_positions() const;
    //this will invalidate the vertex positions array
    void set_vertex_positions(const std::vector<float3>& new_vertex_positions);

    //topology remains unchanged, so we dont need a setter!
    std::vector<uint3> get_triangles() const;
    const std::vector<uint3>& get_triangles_ref() const;


};
