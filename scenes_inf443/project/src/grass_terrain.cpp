#include "grass_terrain.hpp"
#include<iostream>
#include <random>
using namespace cgp;

// Evaluate 3D position of the terrain for any (x,y)
float evaluate_grass_terrain_height(float x, float y)
{
    int points=4;
    vec2 p_0[4]= {{-10,10},{5,5},{-3,4},{6,4}};
    float h_0[4] = {3.0f, -1.5f, 1.0f, 2.0f};
    float sigma_0[4] = {10.0f,3.0f,4.0f,4.0f};
    float z=0.0f;
    for (int i=0;i<points;i++){
    float d = norm(vec2(x, y) - p_0[i]) / sigma_0[i];

    z += 0.2f*h_0[i] * std::exp(-d * d);}

    return z;
}

mesh create_grass_terrain_mesh(int N, float terrain_length)
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
            float z = evaluate_grass_terrain_height(x,y);
            // float persistency = 0.35f;
	        // float frequency_gain = 2.0f;
	        // int octave = 6;
            // float const noise = noise_perlin({u, v}, octave, persistency, frequency_gain);
            // z+=3.0*noise;
            // // Store vertex coordinates
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

bool valid(std::vector<cgp::vec3>trees,cgp::vec3 point){
    float radius=1.0f;
    for (cgp::vec3 p:trees){
        if (norm(p-point)<=radius){
            return false;
        }
    }
    return true;
}
static std::default_random_engine generator(0);
static std::uniform_real_distribution<float> distribution(0,1);
static std::normal_distribution<float> distribution_normal(0, 1);

float rand_uniform(float const value_min, float const value_max)
{
    return distribution(generator)* (value_max-value_min) + value_min;
}
std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_length){
    std::vector<cgp::vec3> pos;
    for (int i=0;i<N;i++){
        


        float x=rand_uniform(-terrain_length/2,terrain_length/2);
        float y=rand_uniform(-terrain_length/2,terrain_length/2);
        float z=evaluate_grass_terrain_height(x,y);
        // float persistency = 0.35f;
	    // float frequency_gain = 2.0f;
	    // int octave = 6;
        // float const noise = noise_perlin({0.5f+x/terrain_length, 0.5f+y/terrain_length}, octave, persistency, frequency_gain);
        // z+=3.0*noise;
        if (valid(pos,cgp::vec3{x,y,z})){
        pos.push_back(cgp::vec3{x,y,z});}
        else{
            i--;
        }
    }
    return pos;
}
