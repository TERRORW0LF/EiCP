#include "cloth_mesh.h"
#include <cassert>
#include <unordered_set>

std::pair<std::vector<float>, std::vector<unsigned int>> read_obj(const std::string &obj_path);

/**
 * @brief Construct a new Cloth Mesh:: Cloth Mesh object
 *
 * @param cloth_path external path to the cloth obj file
 * @param color color of the cloth
 */
ClothMesh::ClothMesh(const std::string &cloth_path, vec3 color)
{
    auto mesh = read_obj(cloth_path);
    std::vector<float> vertices = mesh.first;
    std::vector<unsigned int> faces = mesh.second;

    element_count = faces.size();

    std::vector<float> colors;
    colors.reserve(vertices.size());
    int i = vertices.size() / 3;
    while (i--)
    {
        colors.insert(colors.end(), color.entries, color.entries + 3);
    }

    assert(vertices.size() % 3 == 0);
    vertex_positions.reserve(vertices.size() % 3);
    for (int i = 0; i < vertices.size(); i += 3)
    {
        float3 v;
        v.data[0] = vertices[i];
        v.data[1] = vertices[i + 1];
        v.data[2] = vertices[i + 2];
        vertex_positions.push_back(v);
    }
    vertex_positions_invalid = false;

    assert(faces.size() % 3 == 0);
    triangles.reserve(faces.size() % 3);
    for (int i = 0; i < faces.size(); i += 3)
    {
        uint3 t;
        t.data[0] = faces[i];
        t.data[1] = faces[i + 1];
        t.data[2] = faces[i + 2];
        triangles.push_back(t);
    }
    std::vector<float3> temp_normals;
    compute_normals(temp_normals);


    //construct list of edges
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
    struct KeyHasher
    {
        std::size_t operator()(const RealVector<unsigned int, 2>& k) const
        {
            return ((std::hash<unsigned int>()(k.data[0]) ^ (std::hash<unsigned int>()(k.data[1]) << 1)) >> 1);
        }
    };
    std::unordered_set<RealVector<unsigned int, 2>, KeyHasher> temp_unique_edges;
    for (const uint3& triangle : triangles)
    {
        for (int i = 0; i < 3; i++)
        {
            unsigned int v1 = triangle.data[i];
            unsigned int v2 = triangle.data[(i + 1) % 3];
            RealVector<unsigned int, 2> edge_index;
            edge_index.data[0] = v1;
            edge_index.data[1] = v2;
            if (temp_unique_edges.contains(edge_index))
                continue;

            temp_unique_edges.insert(edge_index);
        }
    }

    double sum_of_distances = 0.0;

    unique_edges.reserve(temp_unique_edges.size());
    for (const auto& a : temp_unique_edges) {
        unique_edges.push_back(a);

        float3 x1 = vertex_positions[a.data[0]];
        float3 x2 = vertex_positions[a.data[1]];
        float3 delta_x1 = x2 - x1;
        float length = delta_x1.length();
        sum_of_distances += length;
    }
    rest_distance = sum_of_distances / (float) unique_edges.size();



    // Holds vertex arrays and their attributes.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Holds a vertex array with its attributes.
    VBOs.resize(3);
    glGenBuffers(3, VBOs.data());

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(float3), vertex_positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Colors
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    // Vertice normals
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, temp_normals.size() * sizeof(float3), temp_normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);

    // Faces buffer. Determines which of the vertices form a triangle.
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(uint3), triangles.data(), GL_STATIC_DRAW);
}

/**
 * @brief Generate and draw the cloth mesh
 *
 */
void ClothMesh::draw()
{
    if (vertex_positions_invalid)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(float3), vertex_positions.data(), GL_STATIC_DRAW);

        compute_and_store_normals();

        vertex_positions_invalid = false;
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, 0);
}

/**
 * @brief Destroy the Cloth Mesh:: Cloth Mesh object
 *
 */
ClothMesh::~ClothMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(3, VBOs.data());
    glDeleteBuffers(1, &EBO);
}

/**
 * @brief Computes and stores normals for the cloth mesh
 *
 */
void ClothMesh::compute_and_store_normals()
{
    std::vector<float3> temp_normals;
    compute_normals(temp_normals);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, temp_normals.size() * sizeof(float3), temp_normals.data(), GL_STATIC_DRAW);
}

/**
 * @brief Computes normals for the cloth mesh
 *
 * @param temp_normals [float3] vector to store the normals
 */
void ClothMesh::compute_normals(std::vector<float3> &temp_normals)
{
    temp_normals.resize(vertex_positions.size(), {0.f, 0.f, 0.f});
    for (const uint3 &t : triangles)
    {
        const float3 &v1 = vertex_positions[t.data[0]];
        const float3 &v2 = vertex_positions[t.data[1]];
        const float3 &v3 = vertex_positions[t.data[2]];

        // compute triangle normal
        float3 e1 = v2 - v1;
        float3 e2 = v3 - v1;

        // compute cross product
        float3 normal = e1.cross_product(e2);

        // compute magnitude
        float magnitude = normal.length();
        normal /= magnitude;

        temp_normals[t.data[0]] += normal;
        temp_normals[t.data[1]] += normal;
        temp_normals[t.data[2]] += normal;
    }

    for (float3 &n : temp_normals)
    {
        float l = n.length();
        n /= l;
    }
}

float ClothMesh::get_rest_distance()
{
    return rest_distance;
}

/**
 * @brief Helper function to get the vertex positions
 *
 * @return std::vector<float3>
 */
std::vector<float3> ClothMesh::get_vertex_positions() const
{
    return vertex_positions;
}

/**
 * @brief Helper function to get the triangles
 *
 * @return std::vector<uint3>
 */
std::vector<uint3> ClothMesh::get_triangles() const
{
    return triangles;
}

/**
 * @brief Helper function to get the vertex positions
 *
 * @return const std::vector<float3>&
 */
const std::vector<uint3> &ClothMesh::get_triangles_ref() const
{
    return triangles;
}

const std::vector<RealVector<unsigned int, 2>>& ClothMesh::get_unique_edges_ref() const
{
    return std::ref(unique_edges);
}

/**
 * @brief Set the vertex positions
 *
 * @param new_vertex_positions
 */
void ClothMesh::set_vertex_positions(const std::vector<float3> &new_vertex_positions)
{
    if (new_vertex_positions.size() != vertex_positions.size())
    {
        std::cout << "error!" << std::endl;
        std::exit(1);
    }

    vertex_positions_invalid = true;
    vertex_positions = new_vertex_positions;
}