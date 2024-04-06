#include "cloth_mesh.h"
#include <cassert>
#include <unordered_set>
#include <cmath>

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
    vertex_positions.reserve(vertices.size() / 3);
    mass.reserve(vertices.size() / 3);
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        vec3 v;
        v.entries[0] = vertices[i];
        v.entries[1] = vertices[i + 1];
        v.entries[2] = vertices[i + 2];
        vertex_positions.push_back(v);
        mass.push_back(0.1f);
    }
    vertex_positions_invalid = false;

    assert(faces.size() % 3 == 0);
    triangles.reserve(faces.size() % 3);
    for (size_t i = 0; i < faces.size(); i += 3)
    {
        uint3 t;
        t.data[0] = faces[i];
        t.data[1] = faces[i + 1];
        t.data[2] = faces[i + 2];
        triangles.push_back(t);
    }
    std::vector<vec3> temp_normals;
    compute_normals(temp_normals);

    // construct list of edges
    //  https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
    struct KeyHasher
    {
        std::size_t operator()(const RealVector<unsigned int, 2> &k) const
        {
            return ((std::hash<unsigned int>()(k.data[0]) ^ (std::hash<unsigned int>()(k.data[1]) << 1)) >> 1);
        }
    };
    std::unordered_set<RealVector<unsigned int, 2>, KeyHasher> temp_unique_edges;
    for (const uint3 &triangle : triangles)
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

    int num_vertices_per_row = std::sqrt(vertex_positions.size());
    for (const auto &a : temp_unique_edges)
    {

        auto v1 = a.data[0];
        auto v2 = a.data[1];

        auto index_dist = std::abs((int)(v1 - v2));

        if (index_dist == 1 || index_dist == num_vertices_per_row)
        {
            unique_springs.push_back(a);
            vec3 x1 = vertex_positions[v1];
            vec3 x2 = vertex_positions[v2];
            vec3 delta_x1 = x2 - x1;
            float length_v = length(delta_x1);
            rest_distance.push_back(length_v);
        }
        else
        {
            continue;
        }
    }

    // Holds vertex arrays and their attributes.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Holds a vertex array with its attributes.
    VBOs.resize(3);
    glGenBuffers(3, VBOs.data());

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(vec3), vertex_positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Colors
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    // Vertice normals
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, temp_normals.size() * sizeof(vec3), temp_normals.data(), GL_STATIC_DRAW);
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
        glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(vec3), vertex_positions.data(), GL_STATIC_DRAW);

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
    std::vector<vec3> temp_normals;
    compute_normals(temp_normals);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, temp_normals.size() * sizeof(vec3), temp_normals.data(), GL_STATIC_DRAW);
}

/**
 * @brief Computes normals for the cloth mesh
 *
 * @param temp_normals [float3] vector to store the normals
 */
void ClothMesh::compute_normals(std::vector<vec3> &temp_normals)
{
    temp_normals.resize(vertex_positions.size(), {0.f, 0.f, 0.f});
    for (const uint3 &t : triangles)
    {
        const vec3 &v1 = vertex_positions[t.data[0]];
        const vec3 &v2 = vertex_positions[t.data[1]];
        const vec3 &v3 = vertex_positions[t.data[2]];

        // compute triangle normal
        vec3 e1 = v2 - v1;
        vec3 e2 = v3 - v1;

        // compute cross product
        vec3 normal = normalize(e1 % e2);

        temp_normals[t.data[0]] += normal;
        temp_normals[t.data[1]] += normal;
        temp_normals[t.data[2]] += normal;
    }

    for (vec3 &n : temp_normals)
    {
        n = normalize(n);
    }
}

/**
 * @returns A pointer to the mass vector.
 *
 * @brief Gets the vector containing particle masses.
 * Each mass is mapped to a particle / vertex by its index.
 */
const std::vector<float> &ClothMesh::get_mass_ref() const
{
    return mass;
}

/**
 * @returns A pointer to the rest distance vector.
 *
 * @brief Gets the vector containing spring rest distances.
 * Each mass is mapped to a spring by its index.
 */
const std::vector<float> &ClothMesh::get_rest_distance_ref() const
{
    return rest_distance;
}

/**
 * @brief Helper function to get the vertex positions
 *
 * @return std::vector<float3>
 */
std::vector<vec3> ClothMesh::get_vertex_positions() const
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

/**
 * @returns A pointer to the spring vector.
 *
 * @brief Gets the vector containing springs.
 * Each spring is unique.
 */
const std::vector<RealVector<unsigned int, 2>> &ClothMesh::get_unique_springs_ref() const
{
    return unique_springs;
}

/**
 * @brief Set the vertex positions
 *
 * @param new_vertex_positions
 */
void ClothMesh::set_vertex_positions(const std::vector<vec3> &new_vertex_positions)
{
    if (new_vertex_positions.size() != vertex_positions.size())
    {
        std::cout << "error!" << std::endl;
        std::exit(1);
    }

    vertex_positions_invalid = true;
    vertex_positions = new_vertex_positions;
}