#version 330 core

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout (location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout (location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout (location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)

// Output variables sent to the fragment shader
out struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal position in world space
    vec3 color;    // vertex color
    vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera

uniform mat4 modelNormal; // Model without scaling used for the normal. modelNormal = transpose(inverse(model))
uniform float time;
uniform vec2 wind;

vec3 deformer(vec3 p0)
{
	float d = sqrt((p0.x-wind.x*time*0.1)*(p0.x-wind.x*time*0.1)+(p0.y-wind.y*time*0.1)*(p0.y-wind.y*time*0.1));
	float omega = 20.0*d - 3.0*time;

	vec3 p = vec3(p0.x, p0.y, 0.05*cos(omega) );

	return p;
}

vec3 deformer_normal(vec3 p0)
{
	float d = sqrt((p0.x-wind.x*time*0.1)*(p0.x-wind.x*time*0.1)+(p0.y-wind.y*time*0.1)*(p0.y-wind.y*time*0.1));
	float omega = 20.0*d - 3.0*time;

	// Compute exact normals after deformation
	vec3 dpdx = vec3(1.0, 0.0, -20.0*abs(p0.x-wind.x*time*0.1)/d*0.05*sin(omega) );
	vec3 dpdy = vec3(0.0, 1.0, -20.0*abs(p0.y-wind.y*time*0.1)/d*0.05*sin(omega) );
	vec3 n = normalize(cross(dpdx,dpdy));

	return n;
}


void main()
{
	// The position of the vertex in the world space
	vec3 p_deformed = deformer(vertex_position);
	vec3 n_deformed = deformer_normal(vertex_position);
	vec4 position = model * vec4(vertex_position, 1.0);

	// The normal of the vertex in the world space
	vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// The projected position of the vertex in the normalized device coordinates:
	vec4 position_projected = projection * view * position;

	// Fill the parameters sent to the fragment shader
	fragment.position = position.xyz;
	fragment.normal   = normal.xyz;
	fragment.color = vertex_color;
	fragment.uv = vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	gl_Position = position_projected; // gl_Position is the projected vertex position (in normalized device coordinates)
}
