#pragma once
#include "cloth_mesh.h"

class SpatialHashStructure
{

public:
	SpatialHashStructure(const std::vector<vec3> &vertices, float spacing, int table_size);

private:
	unsigned int table_size;
	std::vector<unsigned int> table;
	std::vector<unsigned int> particles;
	float spacing;

	unsigned int compute_hash_index(const vec3 &v) const;
	unsigned int hash(int3 index) const;

public:
	std::vector<unsigned int> compute_neighbor_cells(const vec3 &v) const;
	std::pair<unsigned int, unsigned int> get_particle_range_in_cell(unsigned int particle_idx) const;

	inline const std::vector<unsigned int> &get_particles_arr() const
	{
		return particles;
	}
};