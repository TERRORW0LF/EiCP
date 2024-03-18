#include "config.h"
#include "primitives.h"

class ClothMesh
{
public:
    ClothMesh(const std::string& cloth_path, float color[3]);
    void draw();
    ~ClothMesh();

private:
    unsigned int EBO, VAO, element_count;
    std::vector<unsigned int> VBOs;


    mutable bool vertex_positions_invalid = true;
    std::vector<float3> vertex_positions;
public:

    std::vector<float3> get_vertex_positions() const;

    //this will invalidate the vertex positions array
    void set_vertex_positions(const std::vector<float3>& new_vertex_positions);

};
