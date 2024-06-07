#include "snow.hpp"

using namespace cgp;

mesh create_snow(float x,float y, float z,float size,int layer, int angle, unsigned int* index,mesh m){
    if (layer==0){
        m.fill_empty_field();
        return m;
    }
    for (int i=0;i<3;i++){
        m.position.push_back({x+size*sin(2*Pi*i/3+angle*Pi/3),y+size*cos(2*Pi*i/3+angle*Pi/3),z});
    }
    m.connectivity.push_back({uint3{*index,*index+1,*index+2}});
    *index+=3;
    for (int j=0;j<6;j++){
        m=create_snow(x-2.0/3.0*size*sin(2*Pi*j/6+angle*Pi/3),y-2.0/3.0*size*cos(2*Pi*j/6+angle*Pi/3),z,size/3.0,layer-1,angle+j+1,index,m);
    }
    m.color.fill({1.0f,1.0,1.0f});
    m.fill_empty_field();
    return m;
}