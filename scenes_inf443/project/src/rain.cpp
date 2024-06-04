#include "rain.hpp"
using namespace cgp;
mesh create_cylinder_mesh(float radius, float height)
{
    mesh m; 
    int N=20;
    for (int i=0;i<N;++i){
        vec3 point={radius*cos(2*Pi/N*i),radius*sin(2*Pi/N*i),0.0};
        vec3 point2={radius*cos(2*Pi/N*i),radius*sin(2*Pi/N*i),height};
        m.position.push_back(point);
        m.position.push_back(point2);
        m.connectivity.push_back(uint3{(2*i)%(2*N),(2*i+1)%(2*N),(2*i+2)%(2*N)});
        m.connectivity.push_back(uint3{(2*i+3)%(2*N),(2*i+2)%(2*N),(2*i+1)%(2*N)});
    }
 
    // To do: fill this mesh ...
    // ...
    // To add a position: 
    //   m.position.push_back(vec3{x,y,z})
    // Or in pre-allocating the buffer:
    //   m.position.resize(maximalSize);
    //   m.position[index] = vec3{x,y,z}; (with 0<= index < maximalSize)
    // 
    // Similar with the triangle connectivity:
    //  m.connectivity.push_back(uint3{index_1, index_2, index_3});


    // Need to call fill_empty_field() before returning the mesh 
    //  this function fill all empty buffer with default values (ex. normals, colors, etc).
    m.color.fill({0,0,1.0f});
    m.fill_empty_field();

    return m;
}

mesh create_rain()
{
    float h = 0.2f; // trunk height
    float r = 0.01f; // trunk radius
    
    // Create a brown trunk
    mesh droplet = create_cylinder_mesh(r, h);

    return droplet;
}