#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"
#include <deque>
// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI (buttons, etc)
struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;
};

enum BoxType {
	CUBE,
	CYLINDER,
	__FIRST = CUBE,
   	__LAST = CYLINDER
};
struct KeyState {
    bool pressed;
    double press_time;
    double release_time;
	int coor;
	int dir;
};
struct Obj {
	mesh_drawable mesh;
	vec3 pos;
	vec3 vel;
	double size;
};
// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	timer_basic timer;
	std::vector<cgp::vec3> grass_position;
	mesh_drawable terrain;
	mesh_drawable water;
	mesh_drawable grass;
	mesh_drawable tree;
	mesh_drawable cube1;
	mesh_drawable cylinder;
	cgp::hierarchy_mesh_drawable hierarchy;
	struct KeyState state;
	vec3 char_pos;
	vec3 char_vel;
	const double max=5.0f;
	double v_max;
	double v_min;
	vec2 wind;
	int cubeat;
	int point=0;
	int streak=1;
	int cnt=0;
	bool playing;
	// ****************************** //
	// Functions
	// ****************************** //
	std::deque<Obj> cubes;
	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop
	void simulation_step(float dt);
	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();
	void drop_cube();
	void restart();
	mesh_drawable choose_box();
};





