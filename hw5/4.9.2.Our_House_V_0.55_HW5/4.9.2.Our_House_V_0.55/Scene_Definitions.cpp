#define _CRT_SECURE_NO_WARNINGS

#include "Scene_Definitions.h"

unsigned int static_object_ID_mapper[N_MAX_STATIC_OBJECTS];
unsigned int dynamic_object_ID_mapper[N_MAX_DYNAMIC_OBJECTS];
unsigned int camera_ID_mapper[N_MAX_CAMERAS];
unsigned int shader_ID_mapper[N_MAX_SHADERS];

void Axis_Object::define_axis() {
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_axes), &vertices_axes[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void Axis_Object::draw_axis(Shader_Simple* shader_simple, glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix, glm::mat4& ModelMatrix) {	// added modelmatrix outside the function for individual axis per camera
	//glm::mat4 ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
	glm::mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUseProgram(shader_simple->h_ShaderProgram);
	glUniformMatrix4fv(shader_simple->loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);


	glBindVertexArray(VAO);
	glUniform3fv(shader_simple->loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(shader_simple->loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(shader_simple->loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Scene::clock(int clock_id) { // currently one clock
	time_stamp = ++time_stamp % UINT_MAX;
}

void Scene::build_static_world() {
	static_geometry_data.building.define_object();
	static_object_ID_mapper[STATIC_OBJECT_BUILDING] = static_objects.size();
	static_objects.push_back(static_geometry_data.building);

	static_geometry_data.table.define_object();
	static_object_ID_mapper[STATIC_OBJECT_TABLE] = static_objects.size();
	static_objects.push_back(static_geometry_data.table);

	static_geometry_data.light.define_object();
	static_object_ID_mapper[STATIC_OBJECT_LIGHT] = static_objects.size();
	static_objects.push_back(static_geometry_data.light);

	static_geometry_data.teapot.define_object();
	static_object_ID_mapper[STATIC_OBJECT_TEAPOT] = static_objects.size();
	static_objects.push_back(static_geometry_data.teapot);

	static_geometry_data.new_chair.define_object();
	static_object_ID_mapper[STATIC_OBJECT_NEW_CHAIR] = static_objects.size();
	static_objects.push_back(static_geometry_data.new_chair);

	static_geometry_data.frame.define_object();
	static_object_ID_mapper[STATIC_OBJECT_FRAME] = static_objects.size();
	static_objects.push_back(static_geometry_data.frame);

	static_geometry_data.new_picture.define_object();
	static_object_ID_mapper[STATIC_OBJECT_NEW_PICTURE] = static_objects.size();
	static_objects.push_back(static_geometry_data.new_picture);
	/*
	static_geometry_data.cow.define_object();
	static_object_ID_mapper[STATIC_OBJECT_COW] = static_objects.size();
	static_objects.push_back(static_geometry_data.cow);
	*/
	// added from data files
	static_geometry_data.bike.define_object();
	static_object_ID_mapper[STATIC_OBJECT_BIKE] = static_objects.size();
	static_objects.push_back(static_geometry_data.bike);

	static_geometry_data.cat.define_object();
	static_object_ID_mapper[STATIC_OBJECT_CAT] = static_objects.size();
	static_objects.push_back(static_geometry_data.cat);

	static_geometry_data.tank.define_object();
	static_object_ID_mapper[STATIC_OBJECT_TANK] = static_objects.size();
	static_objects.push_back(static_geometry_data.tank);

	static_geometry_data.helicopter.define_object();
	static_object_ID_mapper[STATIC_OBJECT_HELICOPTER] = static_objects.size();
	static_objects.push_back(static_geometry_data.helicopter);

	static_geometry_data.godzilla.define_object();
	static_object_ID_mapper[STATIC_OBJECT_GODZILLA] = static_objects.size();
	static_objects.push_back(static_geometry_data.godzilla);
}

void Scene::build_dynamic_world() {
	/*
	dynamic_geometry_data.tiger_d.define_object();
	dynamic_object_ID_mapper[DYNAMIC_OBJECT_TIGER] = dynamic_objects.size();
	dynamic_objects.push_back(dynamic_geometry_data.tiger_d);

	// two different moving cows
	dynamic_geometry_data.cow_d_1.define_object();
	dynamic_object_ID_mapper[DYNAMIC_OBJECT_COW_1] = dynamic_objects.size();
	dynamic_objects.push_back(dynamic_geometry_data.cow_d_1);

	dynamic_geometry_data.cow_d_2.define_object();
	dynamic_object_ID_mapper[DYNAMIC_OBJECT_COW_2] = dynamic_objects.size();
	dynamic_objects.push_back(dynamic_geometry_data.cow_d_2);
	*/
	// added from data files
	dynamic_geometry_data.nathan_d.define_object();
	dynamic_object_ID_mapper[DYNAMIC_OBJECT_NATHAN] = dynamic_objects.size();
	dynamic_objects.push_back(dynamic_geometry_data.nathan_d);

	dynamic_geometry_data.wolf_d.define_object();
	dynamic_object_ID_mapper[DYNAMIC_OBJECT_WOLF] = dynamic_objects.size();
	dynamic_objects.push_back(dynamic_geometry_data.wolf_d);
}

void Scene::create_camera_list(int win_width, int win_height, float win_aspect_ratio) {
	camera_list.clear();
	// main camera
	camera_data.cam_main.define_camera(win_width, win_height, win_aspect_ratio);
	camera_ID_mapper[CAMERA_MAIN] = camera_list.size();
	camera_list.push_back(camera_data.cam_main);

	// static cc cameras (perspective cams)
	camera_data.cam_cc_0.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_CC_0] = camera_list.size();
	camera_list.push_back(camera_data.cam_cc_0);

	camera_data.cam_cc_1.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_CC_1] = camera_list.size();
	camera_list.push_back(camera_data.cam_cc_1);

	camera_data.cam_cc_2.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_CC_2] = camera_list.size();
	camera_list.push_back(camera_data.cam_cc_2);

	// movable cc camera (perspective cam)
	camera_data.cam_cc_move.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_CC_MOVE] = camera_list.size();
	camera_list.push_back(camera_data.cam_cc_move);

	// static cc cameras (orthographic cams)
	camera_data.cam_front.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_SIDE_FRONT] = camera_list.size();
	camera_list.push_back(camera_data.cam_front);

	camera_data.cam_top.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_TOP] = camera_list.size();
	camera_list.push_back(camera_data.cam_top);

	camera_data.cam_side.define_camera(win_width, win_height, 1.0f);
	camera_ID_mapper[CAMERA_SIDE] = camera_list.size();
	camera_list.push_back(camera_data.cam_side);


	set_active_cc_camera(current_active_cc_cam_id);
}

void Scene::set_active_cc_camera(Camera_ID id) {
	current_active_cc_cam_id = id;

	// update the flags on each cameras so only one camera is rendered
	camera_data.cam_cc_0.flag_valid = (id == CAMERA_CC_0);
	camera_data.cam_cc_1.flag_valid = (id == CAMERA_CC_1);
	camera_data.cam_cc_2.flag_valid = (id == CAMERA_CC_2);
	camera_data.cam_cc_move.flag_valid = (id == CAMERA_CC_MOVE);
	camera_data.cam_front.flag_valid = (id == CAMERA_SIDE_FRONT);
	camera_data.cam_top.flag_valid = (id == CAMERA_TOP);
	camera_data.cam_side.flag_valid = (id == CAMERA_SIDE);
}

void Scene::build_shader_list() {
	shader_data.shader_simple.prepare_shader();
	shader_ID_mapper[SHADER_SIMPLE] = shader_list.size();
	shader_list.push_back(shader_data.shader_simple);

	shader_data.shader_phong.prepare_shader();
	shader_ID_mapper[SHADER_PHONG] = shader_list.size();
	shader_list.push_back(shader_data.shader_phong);

	shader_data.shader_gouraud.prepare_shader();
	shader_ID_mapper[SHADER_GOURAUD] = shader_list.size();
	shader_list.push_back(shader_data.shader_gouraud);
}

void Scene::initialize() {
	axis_object.define_axis();
	build_static_world();
	build_dynamic_world();
	create_camera_list(window.width, window.height, window.aspect_ratio);
	build_shader_list();
}

void Scene::draw_static_world() {
	glm::mat4 ModelViewProjectionMatrix;
	for (auto static_object = static_objects.begin(); static_object != static_objects.end(); static_object++) {
		if (static_object->get().flag_valid == false) continue;
		static_object->get().draw_object(ViewMatrix, ProjectionMatrix, shader_kind, shader_list, lights, n_lights);
	}
}

void Scene::draw_dynamic_world() {
	glm::mat4 ModelViewProjectionMatrix;
	for (auto dynamic_object = dynamic_objects.begin(); dynamic_object != dynamic_objects.end(); dynamic_object++) {
		if (dynamic_object->get().flag_valid == false) continue;
		dynamic_object->get().draw_object(ViewMatrix, ProjectionMatrix, shader_kind, shader_list, time_stamp, lights, n_lights);
	}
}

void Scene::draw_axis() {
#define WC_AXIS_LENGTH		60.0f
	glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
	axis_object.draw_axis(static_cast<Shader_Simple*>(&shader_list[shader_ID_mapper[SHADER_SIMPLE]].get()),
		ViewMatrix, ProjectionMatrix, model_matrix);
}

void Scene::draw_world() {
	draw_axis();
	draw_static_world();
	draw_dynamic_world();
}

void set_material(Material& material, loc_Material& loc_material) {
    glUniform4fv(loc_material.ambient, 1, &material.ambient[0]);
    glUniform4fv(loc_material.diffuse, 1, &material.diffuse[0]);
    glUniform4fv(loc_material.specular, 1, &material.specular[0]); 
    glUniform1f(loc_material.exponent, material.exponent);
    glUniform4fv(loc_material.emission, 1, &material.emission[0]);
}