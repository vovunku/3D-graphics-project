#include "scene.hpp"
#include <GLFW/glfw3.h>

using namespace cgp;

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
	// cube1.model.rotation = rotation_transform::from_axis_angle({ -1,1,0 }, Pi / 7.0f);
	// cube1.model.translation = { 1.0f,1.0f,-0.1f };
	cube1.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/wood.jpg");

	cube2 = cube1;
	mesh_drawable cylinder_base;
	cylinder_base.initialize_data_on_gpu(mesh_primitive_cylinder(0.1f, { 1.0f,0,0 }, { 1.0f,0,0.5f }));
	mesh_drawable sphere;
	sphere.initialize_data_on_gpu(mesh_primitive_sphere(0.1f));
	hierarchy.add(cylinder_base, "Cylinder base");
	hierarchy.add(sphere, "Sphere", "Cylinder base", {1.0f,0,0.6f}); // the translation is used to place the sphere at the extremity of the cylinder
	char_pos={0,0,0};
	char_vel={0.0f,0,0.0f};
}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.

void move_obj(cgp::vec3& pos, cgp::vec3& vel, float dt) {
	//vec3 g = { 0,0,-9.81f }; // gravity
	vel.at(2) -= dt * 9.81;
	for (int i=0;i<3;i++){
	    pos.at(i) = pos.at(i) + dt * vel.at(i);
	}
	if (pos.at(2)<0){
		pos.at(2)=0;
		vel={0,0,0};
	}
}

void scene_structure::simulation_step(float dt)
{
	move_obj(char_pos, char_vel, dt);
	if (cubes.size() > 0) {
		move_obj(cubes.back().pos, cubes.back().vel, dt);
	}
}

void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	// Update time
	timer.update();
	//glfwSetKeyCallback(window.glfw_window, key_callback);
	simulation_step(timer.scale * 0.01f);
	// conditional display of the global frame (set via the GUI)
	hierarchy["Cylinder base"].transform_local.translation = char_pos;
	
	if (gui.display_frame)
		draw(global_frame, environment);

	// Draw all the shapes
	draw(terrain, environment);
	draw(water, environment);
	draw(tree, environment);
	draw(cube1, environment);
	hierarchy.update_local_to_global_coordinates();
	draw(hierarchy, environment);

	if (char_mooving && char_pos.at(2) == 0) {
		char_mooving = false;
		drop_cube();
	}
	for (auto& cube : cubes) {
		cube.mesh.model.translation = cube.pos;
		draw(cube.mesh, environment);
	}


	// Animate the second cube in the water
	cube2.model.translation = { -1.0f, 6.0f+0.1*sin(0.5f*timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	draw(cube2, environment);

	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(water, environment);
		draw_wireframe(tree, environment);
		draw_wireframe(cube1, environment);
		draw_wireframe(cube2, environment);
	}
	







}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

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
	vec3 pos = char_pos;
	pos.at(1) += 2;
	pos.at(2) += 1;
    cubes.push_back({cube1, pos, vec3{0, 0, 0}});
	if (cubes.size() > 10) {
		cubes.pop_front();
	}
}
