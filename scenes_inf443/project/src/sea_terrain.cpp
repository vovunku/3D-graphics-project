#include "sea_terrain.hpp"
#include<iostream>
#include<math.h>
#include "cgp/core/array/array.hpp"
#include "cgp/geometry/vec/vec.hpp"
#include "cgp/geometry/mat/mat.hpp"

using namespace cgp;

// Evaluate 3D position of the terrain for any (x,y)
float evaluate_terrain_height(float x, float y,float t,vec2 wind)
{
    float omega=20*std::sqrt((x-wind.at(0)*t)*(x-wind.at(0)*t)+(y-wind.at(1)*t)*(y-wind.at(1)*t))-3*t;
    float z=0.05*std::cos(omega);
    return z;
}

mesh create_terrain_mesh(int N, float terrain_length,float t, vec2 wind)
{

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N*N);
    // Fill terrain geometry
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the real coordinates (x,y) of the terrain in [-terrain_length/2, +terrain_length/2]
            float x = (u - 0.5f) * terrain_length;
            float y = (v - 0.5f) * terrain_length;

            // Compute the surface height function at the given sampled coordinate
            float z = evaluate_terrain_height(x,y,t,wind);
            //float persistency = 0.35f;
	        //float frequency_gain = 2.0f;
	        //int octave = 6;
            //float const noise = noise_perlin({u, v}, octave, persistency, frequency_gain);
            //z+=3.0*noise;
            // Store vertex coordinates
            terrain.position[kv+N*ku] = {x,y,z};
            terrain.uv[kv+N*ku] = {u,v}; 
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(int ku=0; ku<N-1; ++ku)
    {
        for(int kv=0; kv<N-1; ++kv)
        {
            unsigned int idx = kv + N*ku; // current vertex offset

            uint3 triangle_1 = {idx, idx+1+N, idx+1};
            uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    // need to call this function to fill the other buffer with default values (normal, color, etc)
	terrain.fill_empty_field(); 

    return terrain;
}

void update_terrain_mesh(mesh_drawable terrain,int N,float length, float t, vec2 wind)
{

    //mesh terrain; // temporary terrain storage (CPU only)
    //terrain.position.resize(N*N);
    //terrain.uv.resize(N*N);
    // Fill terrain geometry
    numarray<vec3> pos=numarray<vec3>(N*N);
    numarray<vec2> uv=numarray<vec2>(N*N);
    numarray<uint3> connectivity;
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the real coordinates (x,y) of the terrain in [-terrain_length/2, +terrain_length/2]
            float x = (u - 0.5f) * length;
            float y = (v - 0.5f) * length;

            // Compute the surface height function at the given sampled coordinate
            float z = evaluate_terrain_height(x,y,t,wind);
            //float persistency = 0.35f;
	        //float frequency_gain = 2.0f;
	        //int octave = 6;
            //float const noise = noise_perlin({u, v}, octave, persistency, frequency_gain);
            //z+=3.0*noise;
            // Store vertex coordinates
            pos[kv+N*ku] = {x,y,z};
            uv[kv+N*ku] = {u,v}; 
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    // for(int ku=0; ku<N-1; ++ku)
    // {
    //     for(int kv=0; kv<N-1; ++kv)
    //     {
    //         unsigned int idx = kv + N*ku; // current vertex offset

    //         uint3 triangle_1 = {idx, idx+1+N, idx+1};
    //         uint3 triangle_2 = {idx, idx+N, idx+1+N};

    //         connectivity.push_back(triangle_1);
    //         connectivity.push_back(triangle_2);
    //     }
    // }

    // need to call this function to fill the other buffer with default values (normal, color, etc)
	//terrain.fill_empty_field(); 
    terrain.vbo_position.update(pos);
    terrain.vbo_uv.update(uv);
    //terrain.ebo_connectivity.update(connectivity);
    //return terrain;
}