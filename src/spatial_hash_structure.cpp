#include "spatial_hash_structure.h"
#include <cassert>

/**
 * @brief Construct a new Spatial Hash Structure:: Spatial Hash Structure object
 * 
 * @param vertices The vertices to be discretized
 * @param _spacing The spacing between the cells
 * @param _table_size The size of the table
 */
SpatialHashStructure::SpatialHashStructure(const std::vector<float3> &vertices, float _spacing, int _table_size)
{
	table_size = _table_size + 1;
	spacing = _spacing;
	table.resize(table_size, 0);
	particles.resize(vertices.size(), 0);

	// discretize to bounding box
	for (const float3 &v : vertices)
	{
		unsigned int h = compute_hash_index(v);
		table[h]++;
	}

	// compute the partial sums
	unsigned int sum = 0;
	for (int i = 0; i < table_size; i++)
	{
		sum += table[i];
		table[i] = sum;
	}

	for (int i = 0; i < vertices.size(); i++)
	{
		const float3 &v = vertices[i];
		unsigned int h = compute_hash_index(v);
		unsigned int index = --table[h];

		particles[index] = i;
	}
}

/**
 * @brief Compute the hash index of a vertex
 * 
 * @param v The vertex to be hashed
 * @return unsigned int The hash index
 */
unsigned int SpatialHashStructure::compute_hash_index(const float3 &v)
{
	int x = std::floor(v.data[0] / spacing);
	int y = std::floor(v.data[1] / spacing);
	int z = std::floor(v.data[2] / spacing);
	int3 index3 = {x, y, z};

	unsigned int h = hash(index3);

	return h;
}

/**
 * @brief Hashes an index
 * 
 * @param index index to be hashed
 * @return unsigned int hashed index
 */
unsigned int SpatialHashStructure::hash(int3 index)
{
	int v = (index.data[0] * 92837111) ^ (index.data[1] * 689287499) ^ (index.data[2] * 283923481);
	v = std::abs(v) % (table_size - 1);
	return v;
}

/**
 * @brief Compute the neighboring cells of a vertex
 * 
 * @param v The vertex to compute the neighbors of
 * @return std::vector<unsigned int> The neighboring cells
 */
std::vector<unsigned int> SpatialHashStructure::compute_neighbor_cells(const float3 &v)
{
	std::vector<unsigned int> neighbors;
	neighbors.reserve(27);

	int x = std::floor(v.data[0] / spacing);
	int y = std::floor(v.data[1] / spacing);
	int z = std::floor(v.data[2] / spacing);
	int3 index3 = {x, y, z};

#pragma unroll
	for (int x = index3.data[0] - 1; x < index3.data[0] + 2; x++)
	{
		for (int y = index3.data[1] - 1; y < index3.data[1] + 2; y++)
		{
			for (int z = index3.data[2] - 1; z < index3.data[2] + 2; z++)
			{
				neighbors.push_back(hash(int3{x, y, z}));
			}
		}
	}

	return neighbors;
}

/**
 * @brief Get the particle range in a cell
 * 
 * @param cell_idx The cell index to get the particle range of
 * @return std::pair<unsigned int, unsigned int> The particle range in the cell
 */
std::pair<unsigned int, unsigned int> SpatialHashStructure::get_particle_range_in_cell(unsigned int cell_idx)
{
	assert(cell_idx < table.size() + 1);
	return std::pair<unsigned int, unsigned int>(table[cell_idx], table[cell_idx + 1]);
}
