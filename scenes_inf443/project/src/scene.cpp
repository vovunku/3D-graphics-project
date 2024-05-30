#include "scene.hpp"
#include <GLFW/glfw3.h>
#include <random>

using namespace cgp;
std::random_device rd;          // Obtain a random seed from the hardware
std::mt19937 gen(rd());         // Seed the generator
std::uniform_real_distribution<> dis(0.0, 1.0); // Define the range
void deform_terrain(mesh& m)
{
	// Set the terrain to have a gaussian shape
	for (int k = 0; k < m.position.size(); ++k)
	{
		vec3& p = m.position[k];
		float d2 = p.x*p.x + p.y * p.y;
		float z = exp(-d2 / 4)-1;

		z = z + 0.05f*noise_perlin({ p.x,p.y });

		p = { p.x, p.y, z };
	}

	m.normal_update();
}

mesh_drawable scene_structure::choose_box() {
	BoxType t = static_cast<BoxType>(dis(gen) * (BoxType::__LAST + 1));

	switch (t) {
  	case BoxType::CUBE:
    	return cube1;
  	case BoxType::CYLINDER:
    	return cylinder;
  	default:
    	assert(false);
	}
}

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	std::cout << "Start function scene_structure::initialize()" << std::endl;

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.look_at(
		{ 5.0f, -4.0f, 3.5f } /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);


	// Create the global (x,y,z) frame
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());


	// Create the shapes seen in the 3D scene
	// ********************************************** //

	float L = 5.0f;
	mesh terrain_mesh = mesh_primitive_grid({ -L,-L,0 }, { L,-L,0 }, { L,L,0 }, { -L,L,0 }, 100, 100);
	deform_terrain(terrain_mesh);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg");

	float sea_w = 8.0;
	float sea_z = -0.8f;
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");

	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	cube1.initialize_data_on_gpu(mesh_primitive_cube({ 0,0,0 }, 0.5f));
	//cube1.model.rotation = rotation_transform::from_axis_angle({ -1,1,0 }, Pi / 7.0f);
	cube1.model.translation = { 1.0f,1.0f,-0.1f };
	cube1.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/wood.jpg");

	cylinder.initialize_data_on_gpu(mesh_primitive_cylinder(0.25f, {0, 0, -0.25f}, {0, 0, 0.25f}, 100, 200, true));
	cylinder.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/polkadot.jpg");

	mesh_drawable cylinder_base;
	cylinder_base.initialize_data_on_gpu(mesh_primitive_cylinder(0.1f, { 0,0,-0.25 }, { 0,0,0.25f }));
	mesh_drawable sphere;
	sphere.initialize_data_on_gpu(mesh_primitive_sphere(0.1f));
	hierarchy.add(cylinder_base, "Cylinder base");
	hierarchy.add(sphere, "Sphere", "Cylinder base", {0.0f,0,0.35f}); // the translation is used to place the sphere at the extremity of the cylinder
	char_pos={0,0,0};
	char_vel={0.0f,0,0.0f};
	wind.at(0)=2*max*dis(gen)-max;
	wind.at(1)=2*max*dis(gen)-max;
	cubes.push_back({choose_box(),char_pos,vec3{0,0,0},0.5f});
	cubes.push_back({choose_box(),char_pos+vec3{0,-1,0},vec3{0,0,0},0.5f});

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.

void scene_structure::simulation_step(float dt)
{
	//vec3 g = { 0,0,-9.81f }; // gravity
	if (char_pos.at(2)>0){
	char_vel.at(2) -= dt * 9.81;}
	for (int i=0;i<3;i++){
	    char_pos.at(i) = char_pos.at(i) + dt * char_vel.at(i);
	}
	char_vel.at(state.coor) += wind.at(state.coor) * dt*(char_vel.at(state.coor)-v_min)*(v_max-char_vel.at(state.coor))*char_vel.at(state.coor);
	if (char_pos.at(2)<0){
		char_pos.at(2)=0;
		char_vel={0,0,0};
		wind.at(0)=2*max*dis(gen)-max;
		wind.at(1)=2*max*dis(gen)-max;
		Obj now=cubes.at(0);
		if (char_pos.at(1)-now.pos.at(1)<now.size/2&&char_pos.at(1)-now.pos.at(1)>-now.size/2&&char_pos.at(0)-now.pos.at(0)<now.size/2&&char_pos.at(0)-now.pos.at(0)>-now.size/2){
			streak=1;
		}
		else if (cubeat<cubes.size()){
		Obj c=cubes.at(cubeat);
		if (char_pos.at(1)-c.pos.at(1)<c.size/2&&char_pos.at(1)-c.pos.at(1)>-c.size/2&&char_pos.at(0)-c.pos.at(0)<c.size/2&&char_pos.at(0)-c.pos.at(0)>-c.size/2){
			if (char_pos.at(1)-c.pos.at(1)<c.size/4&&char_pos.at(1)-c.pos.at(1)>-c.size/4&&char_pos.at(0)-c.pos.at(0)<c.size/4&&char_pos.at(0)-c.pos.at(0)>-c.size/4){
				streak+=2;
			}
			else{
				streak=1;
			}
		point+=streak;
		drop_cube();}
		else{
			restart();
		}}
		else{
			restart();
		}
	}
}
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	const vec3 vert={0,0,0.5f};
	// Update time
	timer.update();
	//glfwSetKeyCallback(window.glfw_window, key_callback);
	simulation_step(timer.scale * 0.01f);
	// conditional display of the global frame (set via the GUI)
	hierarchy["Cylinder base"].transform_local.translation = char_pos+vert;
	if (state.pressed){
		//hierarchy["Cylinder base"].transform_local.scaling = 1-0.3*(timer.t-state.press_time);
		hierarchy["Cylinder base"].drawable.model.scaling_xyz={1+0.1*(timer.t-state.press_time),1.0f,1-0.3*(timer.t-state.press_time)};
		hierarchy["Sphere"].drawable.model.translation={0,0,-0.4*0.3*(timer.t-state.press_time)};
	}
	else{
		//hierarchy["Cylinder base"].transform_local.scaling = 1;
		hierarchy["Cylinder base"].drawable.model.scaling_xyz={1.0f,1.0f,1.0f};
		hierarchy["Sphere"].drawable.model.translation={0,0,0};
	}
	if (char_vel.at(2)!=0){
		vec3 rot;
		if (state.coor==0){
			rot={0,-1,0};
		}
		else {
			rot={-1,0,0};
		}
		hierarchy["Cylinder base"].transform_local.rotation = rotation_transform::from_axis_angle(rot, state.dir*1.85*(timer.t-state.release_time)/(state.release_time-state.press_time));
	}
	if (gui.display_frame)
		draw(global_frame, environment);

	// Draw all the shapes
	draw(terrain, environment);
	draw(water, environment);
	draw(tree, environment);
	//draw(cube1, environment);
	hierarchy.update_local_to_global_coordinates();
	draw(hierarchy, environment);
	for (auto& cube : cubes) {
		cube.mesh.model.translation = cube.pos;
		draw(cube.mesh, environment);
	}

	// Animate the second cube in the water
	//cube2.model.translation = { -1.0f, 6.0f+0.1*sin(0.5f*timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	//cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	//draw(cube2, environment);

	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(water, environment);
		draw_wireframe(tree, environment);
		//draw_wireframe(cube1, environment);
		//draw_wireframe(cube2, environment);
	}
}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::Text("wind on x-axis %f", wind.at(0));
	ImGui::Text("wind on y-axis %f", wind.at(1));
	ImGui::Text("Current score %d", point);

}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}
void scene_structure::drop_cube()
{
	
	Obj c=cubes.at(cubeat);
	cubes.clear();
	cubes.push_back(c);
	for (int i=0;i<4;i++){
		vec3 pos = c.pos;
		double a=dis(gen);
		if (i==0 || i==3){
			a=-a;
		}
		pos.at((i+1)%2)+=5.0f*a;
		cubes.push_back({choose_box(), pos, vec3{0, 0, 0},0.5f});
		if (dis(gen)<0.25*(i+1)){
			break;
		}
	}
}
void scene_structure::restart()
{
	streak=1;
	point=0;
	char_pos={0,0,0};
	char_vel={0.0f,0,0.0f};
	wind.at(0)=2*max*dis(gen)-max;
	wind.at(1)=2*max*dis(gen)-max;
	cubes.clear();
	cubes.push_back({choose_box(),char_pos,vec3{0,0,0},0.5f});
	cubes.push_back({choose_box(),char_pos+vec3{0,-1,0},vec3{0,0,0},0.5f});
}


