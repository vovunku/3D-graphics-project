#pragma once

#include "cgp/cgp.hpp"

using namespace cgp;
float evaluate_sea_terrain_height(float x, float y,float t,vec2 wind);

/** Compute a terrain mesh 
	The (x,y) coordinates of the terrain are set in [-length/2, length/2].
	The z coordinates of the vertices are computed using evaluate_terrain_height(x,y).
	The vertices are sampled along a regular grid structure in (x,y) directions. 
	The total number of vertices is N*N (N along each direction x/y) 	*/
cgp::mesh create_sea_terrain_mesh(int N, float length,float t,vec2 wind);
void update_sea_terrain_mesh(mesh_drawable terrain,int N,float length, float t, vec2 wind);