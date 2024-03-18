#include "cloth_mesh.h"
#include <cassert>
#include "algebraic_types.h"

std::pair<std::vector<float>, std::vector<unsigned int>> readObj(const std::string &obj_path);

ClothMesh::ClothMesh(const std::string &cloth_path, float color[3])
{
    auto mesh = readObj(cloth_path);
    std::vector<float> vertices = mesh.first;
    std::vector<unsigned int> faces = mesh.second;

    element_count = faces.size();

    std::vector<float> colors;
    colors.reserve(vertices.size());
    int i = vertices.size() / 3;
    while (i--)
    {
        colors.insert(colors.end(), color, color + 3);
    }


    assert(vertices.size() % 3 == 0);
    vertex_positions.reserve(vertices.size() % 3);
    for (int i = 0; i < vertices.size(); i += 3) {
        float3 v;
        v.data[0] = vertices[i];
        v.data[1] = vertices[i + 1];
        v.data[2] = vertices[i + 2];
        vertex_positions.push_back(v);
    }
    vertex_positions_invalid = false;

    assert(faces.size() % 3 == 0);
    triangles.reserve(faces.size() % 3);
    for (int i = 0; i < faces.size(); i += 3) {
        uint3 t;
        t.data[0] = faces[i];
        t.data[1] = faces[i+1];
        t.data[2] = faces[i+2];
        triangles.push_back(t);
    }

    std::vector<float3> tempNormals;
    tempNormals.resize(vertex_positions.size(), { 0.f,0.f,0.f });
    for (const uint3& t : triangles) {
        const float3& v1 = vertex_positions[t.data[0]];
        const float3& v2 = vertex_positions[t.data[1]];
        const float3& v3 = vertex_positions[t.data[2]];

        //compute triangle normal
        
        float3 e1 = v1 - v2;

        float3 e2 = v1- v3;

        //compute cross product
        float3 normal = e1.cross_product(e2);

        //compute magnitude
        float magnitude = normal.length();
        normal /= magnitude;

        tempNormals[t.data[0]] += normal;
        tempNormals[t.data[1]] += normal;
        tempNormals[t.data[2]] += normal;
    }

    for (float3& n : tempNormals) {
        float l = n.length();
        n /= l;
    }


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBOs.resize(3);
    glGenBuffers(3, VBOs.data());

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(float3), vertex_positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, tempNormals.size() * sizeof(float3), tempNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(uint3), triangles.data(), GL_STATIC_DRAW);

}

void ClothMesh::draw()
{
    if (vertex_positions_invalid) {
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(float3), vertex_positions.data(), GL_STATIC_DRAW);

        vertex_positions_invalid = false;
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, 0);
}

ClothMesh::~ClothMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBOs.data());
    glDeleteBuffers(1, &EBO);
}

std::vector<float3> ClothMesh::get_vertex_positions() const
{
    return vertex_positions;
}

std::vector<uint3> ClothMesh::get_triangles() const
{
    return triangles;
}

const std::vector<uint3>& ClothMesh::get_triangles_ref() const
{
    return triangles;
}

void ClothMesh::set_vertex_positions(const std::vector<float3>& new_vertex_positions)
{
    if (new_vertex_positions.size() != vertex_positions.size()) {
        std::cout << "error!" << std::endl;
        std::exit(1);
    }
       
    vertex_positions_invalid = true;
    vertex_positions = new_vertex_positions;
}

std::pair<std::vector<float>, std::vector<unsigned int>> readObj(const std::string &obj_path)
{
    std::vector<float> vertices;
    std::vector<unsigned int> faces;

    // Setup streams for reading obj file.
    std::ifstream file;
    std::string line;

    // Try to open the shader file. Print warning if that failed.
    file.open(obj_path);
    if (!file.is_open())
    {
        std::cout << "Unable to open obj file." << std::endl;
    }

    // Read file and convert it into a char array.
    while (std::getline(file, line))
    {
        std::stringstream line_stream(line);
        std::string prefix;
        float values[3];
        while (line_stream >> prefix)
        {
            if (prefix == "v")
            {
                line_stream >> values[0] >> values[1] >> values[2];
                vertices.insert(vertices.end(), values, values + 3);
            }
            else if (prefix == "f")
            {
                line_stream >> values[0] >> values[1] >> values[2];
                values[0] -= 1;
                values[1] -= 1;
                values[2] -= 1;
                faces.insert(faces.end(), values, values + 3);
            }
        }
    }

    return std::make_pair(vertices, faces);
}