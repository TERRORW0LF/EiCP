#include "config.h"

class ClothMesh
{
public:
    ClothMesh(const std::string &cloth_path, float color[3]);
    void draw();
    ~ClothMesh();

private:
    unsigned int EBO, VAO, element_count;
    std::vector<unsigned int> VBOs;
};