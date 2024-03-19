#pragma once
#include "cloth_mesh.h"

class SpatialHashStructure
{

public:
	SpatialHashStructure(const std::vector<float3> &vertices, float spacing, int table_size);

private:
	unsigned int table_size;
	std::vector<unsigned int> table;
	std::vector<unsigned int> particles;
	float spacing;

	unsigned int compute_hash_index(const float3 &v);
	unsigned int hash(int3 index);

public:
	std::vector<unsigned int> compute_neighbor_cells(const float3 &v);
	std::pair<unsigned int, unsigned int> get_particle_range_in_cell(unsigned int particle_idx);

	inline const std::vector<unsigned int> &get_particles_arr()
	{
		return particles;
	}
};