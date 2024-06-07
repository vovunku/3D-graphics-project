#include "scene.hpp"
#include <GLFW/glfw3.h>
#include <random>
#include <thread>  // Include the thread library
#include <chrono>  // Include the chrono library for duration

#include "sea_terrain.hpp"
#include "grass_terrain.hpp"
#include "rain.hpp"
#include "snow.hpp"
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
		return cube1;
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

	//float L = 5.0f;
	// mesh terrain_mesh = mesh_primitive_grid({ -L,-L,0 }, { L,-L,0 }, { L,L,0 }, { -L,L,0 }, 100, 100);
	// deform_terrain(terrain_mesh);
	// terrain.initialize_data_on_gpu(terrain_mesh);
	// terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg");

	float sea_w = 8.0;
	int N_terrain_samples = 100;
	//float terrain_length = 20;
	mesh terrain_sea=create_sea_terrain_mesh(N_terrain_samples,2*sea_w,0,{0,0});
	//water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	water.initialize_data_on_gpu(terrain_sea);
	water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");
	opengl_shader_structure shader_custom;
	shader_custom.load(
    project::path + "shaders/mesh/mesh_moving.vert.glsl", 
    project::path + "shaders/mesh/mesh.frag.glsl");
	water.shader=shader_custom;
	// int N_terrain_samples = 100;
	//float terrain_length = 20;
	mesh terrain_grass=create_grass_terrain_mesh(N_terrain_samples,2*sea_w);
	//water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	terrain.initialize_data_on_gpu(terrain_grass);
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/texture_grass.jpg");
	grass_position=generate_positions_on_terrain(N_terrain_samples,2*sea_w);
	mesh quad_mesh = mesh_primitive_quadrangle({ -0.5f,0,0 }, { 0.5f,0,0 }, { 0.5f,0,1 }, { -0.5f,0,1 });
	grass.initialize_data_on_gpu(quad_mesh);
	grass.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/grass.png");
	opengl_shader_structure shader_custom_grass;
	shader_custom_grass.load(
    project::path + "shaders/mesh/mesh_grass.vert.glsl", 
    project::path + "shaders/mesh/mesh.frag.glsl");
	grass.shader=shader_custom_grass;
	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	cube1.initialize_data_on_gpu(mesh_primitive_cube({ 0,0,0 }, 0.5f));
	//cube1.model.rotation = rotation_transform::from_axis_angle({ -1,1,0 }, Pi / 7.0f);
	cube1.model.translation = { 1.0f,1.0f,-0.1f };
	cube1.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/wood.jpg");

	cylinder.initialize_data_on_gpu(mesh_primitive_cylinder(0.25f, {0, 0, -0.25f}, {0, 0, 0.25f}, 100, 200, true));
	cylinder.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/polkadot.jpg");
	mesh rain=create_rain();
	raindrop.initialize_data_on_gpu(rain);
	mesh snow;
	unsigned int index=0;
	snow=create_snow(0,0,0.0f,0.25f,4,0,&index,snow);
	snowflake.initialize_data_on_gpu(snow);
	opengl_shader_structure shader_custom_rain;
	shader_custom_rain.load(
    project::path + "shaders/mesh/mesh_rain.vert.glsl", 
    project::path + "shaders/mesh/mesh.frag.glsl");
	raindrop.shader=shader_custom_rain;
	mesh_drawable cylinder_base;
	cylinder_base.initialize_data_on_gpu(mesh_primitive_cylinder(0.1f, { 0,0,-0.25 }, { 0,0,0.25f }));
	mesh_drawable sphere;
	sphere.initialize_data_on_gpu(mesh_primitive_sphere(0.1f));
	hierarchy.add(cylinder_base, "Cylinder base");
	hierarchy.add(sphere, "Sphere", "Cylinder base", {0.0f,0,0.35f}); // the translation is used to place the sphere at the extremity of the cylinder
	char_pos={0,0,0};
	char_vel={0.0f,0,0.0f};
	wind.at(0)=0;
	wind.at(1)=0;
	cubes.push_back({choose_box(), char_pos,vec3{0,0,0}, 0.5f});
	cubes.push_back({choose_box(), char_pos+vec3{0,-1,0}, vec3{0,0,0}, 0.5f});
	playing=false;
}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.

void scene_structure::simulation_step(float dt)
{
	//vec3 g = { 0,0,-9.81f }; // gravity
	int difficulty=cnt/5;
		if (difficulty>=5){
			difficulty=5;
		}
	if (dis(gen)<=difficulty*0.1){
		float sea_w = 8.0;
		float x= 2*sea_w*dis(gen)-sea_w;
		float y= 2*sea_w*dis(gen)-sea_w;
		droplets.push_back({raindrop,vec3{x,y,10},vec3{0,0,0},0.2f});
		//std::cout<<"1"<<std::endl;
	}
	if (dis(gen)<=difficulty*0.1){
		float sea_w = 8.0;
		float x= 2*sea_w*dis(gen)-sea_w;
		float y= 2*sea_w*dis(gen)-sea_w;
		snowdrops.push_back({snowflake,vec3{x,y,10},vec3{0,0,0},0.2f});
		//std::cout<<"1"<<std::endl;
	}
	for (int i=0;i<droplets.size();i++){
		droplets.at(i).pos+=droplets.at(i).vel*dt;
		droplets.at(i).vel.at(0)+=wind.at(0)*dt;
		droplets.at(i).vel.at(1)+=wind.at(1)*dt;
		droplets.at(i).vel.at(2)-=9.81f*dt;
		//std::cout<<droplets.at(i).pos.at(2)<<std::endl;
		if (droplets.at(i).pos.at(2)<0){
			droplets.erase(droplets.begin()+i);
			i--;
		}
	}
	for (int i=0;i<snowdrops.size();i++){
		snowdrops.at(i).pos+=snowdrops.at(i).vel*dt/10;
		snowdrops.at(i).vel.at(0)+=wind.at(0)*dt;
		snowdrops.at(i).vel.at(1)+=wind.at(1)*dt;
		snowdrops.at(i).vel.at(2)-=9.81f*dt;
		if (snowdrops.at(i).pos.at(2)<0){
			snowdrops.erase(snowdrops.begin()+i);
			i--;
		}
	}
	if (char_pos.at(2)>0){
	char_vel.at(2) -= dt * 9.81;}
	for (int i=0;i<3;i++){
	    char_pos.at(i) = char_pos.at(i) + dt * char_vel.at(i);
	}
	char_vel.at(state.coor) += wind.at(state.coor) * dt*(char_vel.at(state.coor)-v_min)*(v_max-char_vel.at(state.coor))*char_vel.at(state.coor)*0.0001;
	if (char_pos.at(2)<0){
		char_pos.at(2)=0;
		char_vel={0,0,0};
		wind.at(0)=2*difficulty*dis(gen)-difficulty;
		wind.at(1)=2*difficulty*dis(gen)-difficulty;
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
		cnt+=1;
		if (!animation){
		drop_cube();}
		else{
			float t=(difficulty+1)*0.1f*dis(gen)*timer.scale+0.5f;
			state.press_time=timer.t+2.0f*timer.scale;
			v_max=4.0f*(state.release_time-state.press_time)*state.dir;
			v_min=2.0f*(state.release_time-state.press_time)*state.dir;
			state.pressed=false;
			state.release_time=state.press_time+t;
			state.coor=0;
			state.dir=1;
			if (dis(gen)>0.5){
				state.coor=1;
			}
			if (dis(gen)>0.5){
				state.dir=-1;
			}
			float a=t*state.dir;
			animate(0.01*timer.scale,state.coor,a);
		}}
		else{
			playing=false;
			//restart();
		}}
		else{
			playing=false;
			//restart();
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
	environment.uniform_generic.uniform_float["time"] = timer.t;
	environment.uniform_generic.uniform_vec2["wind"] = wind;
	environment.uniform_generic.uniform_float["count"] = cnt;
	environment.uniform_generic.uniform_vec3["charpos"] = char_pos;
	environment.background_color=vec3{0,1.0,1.0};
	//glfwSetKeyCallback(window.glfw_window, key_callback);
	
	if (animation){
		if (timer.t>=state.release_time && state.pressed){
			state.pressed=false;
			char_vel.at(2)=10.0f*(state.release_time-state.press_time);
			char_vel.at(state.coor)=3.0f*(state.release_time-state.press_time)*state.dir;
			
		}
	}
	simulation_step(timer.scale * 0.01f);
	if (animation){
		if (timer.t>=state.press_time && timer.t<state.release_time){
			state.pressed=true;
		}
	}
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
	// int newzoom=(int)((log2(zoom)));
	// if (newzoom != oldzoom){
	// 	oldzoom = newzoom;
	// 	snowflake.clear();
	// 	unsigned int index=0;
	// 	mesh snow;
	// 	int nblayers=4+newzoom;
	// 	if (nblayers<1){
	// 		nblayers=1;
	// 	}
	// 	snow=create_snow(0,0,5.0f,1.0f,nblayers,0,&index,snow);
	// 	snowflake.initialize_data_on_gpu(snow);
	// }
	vec3 rot;
		if (state.coor==0){
			rot={0,-1,0};
		}
		else {
			rot={-1,0,0};
		}
	if (char_vel.at(2)!=0){
		
		hierarchy["Cylinder base"].transform_local.rotation = rotation_transform::from_axis_angle(rot, state.dir*1.85*(timer.t-state.release_time)/(state.release_time-state.press_time));
	}
	else{
		hierarchy["Cylinder base"].transform_local.rotation = rotation_transform::from_axis_angle(rot, 0);
	}
	if (gui.display_frame)
		draw(global_frame, environment);

	// Draw all the shapes
	//draw(terrain, environment);
	// int N_terrain_samples=100;
	// float sea_w=8.0;
	//update_terrain_mesh(water,N_terrain_samples,2*sea_w,timer.t,wind);
	//water.clear();
	// mesh terrain_sea=create_terrain_mesh(N_terrain_samples,2*sea_w,timer.t,wind);
	// //water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	// water.initialize_data_on_gpu(terrain_sea);
	// water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");
	if (num_scene==1){
	draw(water, environment);}
	if (num_scene==0){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable depth buffer writing
	//  - Transparent elements cannot use depth buffer
	//  - They are supposed to be display from furest to nearest elements
	glDepthMask(false);
	draw(terrain,environment);
	//draw(tree, environment);
	//draw(cube1, environment);
	
	for (vec3 pos:grass_position){
	auto const& camera = camera_control.camera_model;

	// Re-orient the grass shape to always face the camera direction
	vec3 const right = camera.right();
	// Rotation such that the grass follows the right-vector of the camera, while pointing toward the z-direction
	rotation_transform R = rotation_transform::from_frame_transform({ 1,0,0 }, { 0,0,1 }, right, { 0,0,1 });
	grass.model.rotation = R;
	grass.model.translation=pos;
	draw(grass, environment);
	if (gui.display_wireframe)
		draw_wireframe(grass,environment);}
	// Animate the second cube in the water
	//cube2.model.translation = { -1.0f, 6.0f+0.1*sin(0.5f*timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	//cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	//draw(cube2, environment);
	if (gui.display_wireframe) {
		//draw_wireframe(terrain, environment);
		draw_wireframe(water, environment);
		//draw_wireframe(tree, environment);
		//draw_wireframe(cube1, environment);
		//draw_wireframe(cube2, environment);
	}
	glDepthMask(true);
	glDisable(GL_BLEND);}
	hierarchy.update_local_to_global_coordinates();
	draw(hierarchy, environment);
	for (auto& cube : cubes) {
		cube.mesh.model.translation = cube.pos;
		cube.mesh.model.scaling_xyz.at(0)=cube.size*2.0f;
		cube.mesh.model.scaling_xyz.at(1)=cube.size*2.0f;
		draw(cube.mesh, environment);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable depth buffer writing
	//  - Transparent elements cannot use depth buffer
	//  - They are supposed to be display from furest to nearest elements
	glDepthMask(false);
		for (auto rain:droplets){
		rain.mesh.model.translation = rain.pos;
		draw(rain.mesh, environment);
		//std::cout<<"yes"<<std::endl;
	}
	for (auto snow:snowdrops){
		double theta=2*Pi*dis(gen);
		double phi=2*Pi*dis(gen);
		vec3 ax={cos(theta),sin(theta)*cos(phi),sin(theta)*cos(phi)};
		snow.mesh.hierarchy_transform_model.rotation=rotation_transform::from_axis_angle(ax,timer.t/300);
		snow.mesh.hierarchy_transform_model.translation = snow.pos;
		draw(snow.mesh, environment);
		//std::cout<<"yes"<<std::endl;
	}
	// if (snowdrops.size()!=0){
	// 	Obj snow=snowdrops.at(0);
	// 	double theta=2*Pi*dis(gen);
	// 	double phi=2*Pi*dis(gen);
	// 	vec3 ax={cos(theta),sin(theta)*cos(phi),sin(theta)*cos(phi)};
	// 	//snowflake.hierarchy_transform_model.rotation=rotation_transform::from_axis_angle(ax,timer.t/1000);
	// 	snowflake.hierarchy_transform_model.translation = snow.pos;
	// 	draw(snowflake, environment);
	// }
	

	glDepthMask(true);
	glDisable(GL_BLEND);
	
	//draw(snowflake,environment);
}

void scene_structure::display_gui()
{
	//ImGui::Checkbox("Frame", &gui.display_frame);
	//ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::Text("wind on x-axis %f", wind.at(0));
	ImGui::Text("wind on y-axis %f", wind.at(1));
	ImGui::Text("Current score %d", point);
	int difficulty=cnt/5;
		if (difficulty>=5){
			difficulty=5;
		}
	ImGui::Text("Current dfficulty %d", difficulty);
	//ImGui::Text("Pressed time %f Released time %f now %f", state.press_time,state.release_time,timer.t);
	if (!playing){
		if (!firstvisit){
		ImGui::Text("Game Over!!!");}
	}
	bool restarting=ImGui::Button("New Game");
	if (restarting){
		animation=false;
		firstvisit=false;
		restart();
	}
	bool grass_scene=ImGui::Button("Grass");
	if (grass_scene){
		num_scene=0;
	}
	bool sea_scene=ImGui::Button("Sea");
	if (sea_scene){
		num_scene=1;
	}
	bool switching = false;
	if (!animation){
	switching=ImGui::Button("Animation");}
	if (switching){
		firstvisit=false;
		animation=!animation;
		if (animation){
			restart();
			cubeat=0;
			float t=(difficulty+1)*0.1f*dis(gen)*timer.scale+0.5f;
			state.press_time=timer.t+2.0f*timer.scale;
			state.pressed=false;
			state.release_time=state.press_time+t;
			state.coor=0;
			state.dir=1;
			if (dis(gen)>0.5){
				state.coor=1;
			}
			if (dis(gen)>0.5){
				state.dir=-1;
			}
			float a=t*state.dir;
			animate(0.01f*timer.scale,state.coor,a);
		}
		else {
			restart();
		}
	}
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
	int difficulty=cnt/5;
		if (difficulty>=5){
			difficulty=5;
		}
	for (int i=0;i<4;i++){
		vec3 pos = char_pos;
		
		double a=1+(difficulty+1)*dis(gen);
		if (i==0 || i==3){
			a=-a;
		}
		pos.at((i+1)%2)+=a;
		cubes.push_back({choose_box(), pos, vec3{0, 0, 0},0.5f*(1.0f-difficulty*0.1f)});
		if (dis(gen)<0.25*(i+1)){
			break;
		}
	}
}
void scene_structure::restart()
{

	//std::this_thread::sleep_for(std::chrono::seconds(5));
	streak=1;
	point=0;
	cnt=0;
	char_pos={0,0,0};
	char_vel={0.0f,0,0.0f};
	wind.at(0)=0;
	wind.at(1)=0;
	cubes.clear();
	cubes.push_back({choose_box(),char_pos,vec3{0,0,0},0.5f});
	cubes.push_back({choose_box(),char_pos+vec3{0,-1,0},vec3{0,0,0},0.5f});
	playing=true;
}
int scene_structure::find(int dir,float a){
	if (a<0){
		if (dir==0){
			return 3;
		}
		else{
			return 0;
		}
	}
	else{
		if (dir==0){
			return 1;
		}
		else{
			return 2;
		}
	}
}
void scene_structure::animate(float dt,int dir,float a){
	vec3 temp_pos;
	vec3 temp_vel;
	for (int i=0;i<3;i++){
		temp_pos.at(i)=char_pos.at(i);
	}
	// char_vel.at(dir)=3*a;
	// char_vel.at(1-dir)=0;
	// char_vel.at(2)=10*fabs(a);
	temp_vel.at(dir)=3*a;
	temp_vel.at(1-dir)=0;
	temp_vel.at(2)=10*fabs(a);
	while(temp_pos.at(2)+temp_vel.at(2)*dt>=0){
		temp_vel.at(2) -= dt * 9.81;
		for (int i=0;i<3;i++){
	    	temp_pos.at(i) = temp_pos.at(i) + dt * temp_vel.at(i);
		}
		temp_vel.at(state.coor) += wind.at(state.coor) * dt*(temp_vel.at(state.coor)-v_min)*(v_max-temp_vel.at(state.coor))*temp_vel.at(state.coor)*0.0001;
	}
	Obj c=cubes.at(cubeat);
	cubes.clear();
	cubes.push_back(c);
	int difficulty=cnt/5;
		if (difficulty>=5){
			difficulty=5;
		}
	
	
	cubeat=find(dir,a)+1;
	for (int i=0;i<cubeat-1;i++){
		vec3 pos = char_pos;
		double a=1+(difficulty+1)*dis(gen);
		if (i==0 || i==3){
			a=-a;
		}
		pos.at((i+1)%2)+=a;
		cubes.push_back({choose_box(), pos, vec3{0, 0, 0},0.5f*(1.0f-difficulty*0.1f)});
	}
	cubes.push_back({choose_box(),temp_pos,vec3{0,0,0},0.5f*(1.0f-difficulty*0.1f)});

}

