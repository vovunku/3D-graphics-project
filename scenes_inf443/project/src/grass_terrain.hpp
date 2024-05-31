#pragma once

#include "cgp/cgp.hpp"


float evaluate_grass_terrain_height(float x, float y);

/** Compute a terrain mesh 
	The (x,y) coordinates of the terrain are set in [-length/2, length/2].
	The z coordinates of the vertices are computed using evaluate_terrain_height(x,y).
	The vertices are sampled along a regular grid structure in (x,y) directions. 
	The total number of vertices is N*N (N along each direction x/y) 	*/
cgp::mesh create_grass_terrain_mesh(int N, float length);
bool valid(std::vector<cgp::vec3>,cgp::vec3 point);
float rand_uniform(float const value_min, float const value_max);
std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_length);

