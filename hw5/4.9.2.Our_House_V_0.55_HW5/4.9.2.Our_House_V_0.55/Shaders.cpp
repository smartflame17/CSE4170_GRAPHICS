#include "Scene_Definitions.h"

void Shader_Simple::prepare_shader() {
	shader_info[0] = { GL_VERTEX_SHADER, "Shaders/simple.vert" };
	shader_info[1] = { GL_FRAGMENT_SHADER, "Shaders/simple.frag" };
	shader_info[2] = { GL_NONE, NULL };

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
	glUseProgram(0);
}

void Shader_Phong::prepare_shader() {
	shader_info[0] = { GL_VERTEX_SHADER, "Shaders/phong.vert" };
	shader_info[1] = { GL_FRAGMENT_SHADER, "Shaders/phong.frag" };
	shader_info[2] = { GL_NONE, NULL };

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans = glGetUniformLocation(h_ShaderProgram, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram, "u_global_ambient_color");

	// material related
	loc_material.ambient = glGetUniformLocation(h_ShaderProgram, "u_material.ambient_color");
	loc_material.diffuse = glGetUniformLocation(h_ShaderProgram, "u_material.diffuse_color");
	loc_material.specular = glGetUniformLocation(h_ShaderProgram, "u_material.specular_color");
	loc_material.emission = glGetUniformLocation(h_ShaderProgram, "u_material.emissive_color");
	loc_material.exponent = glGetUniformLocation(h_ShaderProgram, "u_material.specular_exponent");


	// light related
	loc_n_lights = glGetUniformLocation(h_ShaderProgram, "u_n_lights");
	for (int i = 0; i < N_MAX_LIGHTS; i++) {
		char uniform_name[256];
		sprintf(uniform_name, "u_lights[%d].light_on", i);
		loc_lights[i].light_on = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].position", i);
		loc_lights[i].position = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].ambient_color", i);
		loc_lights[i].ambient_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].diffuse_color", i);
		loc_lights[i].diffuse_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].specular_color", i);
		loc_lights[i].specular_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_direction", i);
		loc_lights[i].spot_direction = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_exponent", i);
		loc_lights[i].spot_exponent = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_cutoff_angle", i);
		loc_lights[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].light_attenuation_factors", i);
		loc_lights[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram, uniform_name);
	}

	glUseProgram(0);
}

void Shader_Gouraud::prepare_shader() {
	shader_info[0] = { GL_VERTEX_SHADER, "Shaders/gouraud.vert" };
	shader_info[1] = { GL_FRAGMENT_SHADER, "Shaders/gouraud.frag" };
	shader_info[2] = { GL_NONE, NULL };

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans = glGetUniformLocation(h_ShaderProgram, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram, "u_global_ambient_color");

	// material related
	loc_material.ambient = glGetUniformLocation(h_ShaderProgram, "u_material.ambient_color");
	loc_material.diffuse = glGetUniformLocation(h_ShaderProgram, "u_material.diffuse_color");
	loc_material.specular = glGetUniformLocation(h_ShaderProgram, "u_material.specular_color");
	loc_material.emission = glGetUniformLocation(h_ShaderProgram, "u_material.emissive_color");
	loc_material.exponent = glGetUniformLocation(h_ShaderProgram, "u_material.specular_exponent");


	// light related
	loc_n_lights = glGetUniformLocation(h_ShaderProgram, "u_n_lights");
	for (int i = 0; i < N_MAX_LIGHTS; i++) {
		char uniform_name[256];
		sprintf(uniform_name, "u_lights[%d].light_on", i);
		loc_lights[i].light_on = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].position", i);
		loc_lights[i].position = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].ambient_color", i);
		loc_lights[i].ambient_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].diffuse_color", i);
		loc_lights[i].diffuse_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].specular_color", i);
		loc_lights[i].specular_color = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_direction", i);
		loc_lights[i].spot_direction = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_exponent", i);
		loc_lights[i].spot_exponent = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].spot_cutoff_angle", i);
		loc_lights[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram, uniform_name);
		sprintf(uniform_name, "u_lights[%d].light_attenuation_factors", i);
		loc_lights[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram, uniform_name);
	}

	glUseProgram(0);
}