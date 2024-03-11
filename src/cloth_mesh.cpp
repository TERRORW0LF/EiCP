#include "cloth_mesh.h"

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

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBOs.resize(2);
    glGenBuffers(2, VBOs.data());

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(int), faces.data(), GL_STATIC_DRAW);
}

void ClothMesh::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, element_count, GL_UNSIGNED_INT, 0);
}

ClothMesh::~ClothMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBOs.data());
    glDeleteBuffers(1, &EBO);
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