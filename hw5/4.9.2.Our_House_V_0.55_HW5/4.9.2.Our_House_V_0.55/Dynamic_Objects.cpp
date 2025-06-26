#define _CRT_SECURE_NO_WARNINGS

#include "Scene_Definitions.h"

void Tiger_D::define_object() {
#define N_TIGER_FRAMES 12
	glm::mat4* cur_MM;
	Material* cur_material;
	flag_valid = true;

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		object_frames.emplace_back();
		sprintf(object_frames[i].filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		object_frames[i].n_fields = 8;
		object_frames[i].front_face_mode = GL_CW;
		object_frames[i].prepare_geom_of_static_object();

		object_frames[i].instances.emplace_back();
		cur_MM = &(object_frames[i].instances.back().ModelMatrix);
		*cur_MM = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		cur_material = &(object_frames[i].instances.back().material);
		cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		cur_material->ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
		cur_material->diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
		cur_material->specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
		cur_material->exponent = 128.0f * 0.21794872f;
	}
}

void Cow_D::define_object() {
#define N_FRAMES_COW_1 1
#define N_FRAMES_COW_2 1
	glm::mat4* cur_MM;
	Material* cur_material;
	flag_valid = true;
	switch (object_id) {

		int n_frames;
	case DYNAMIC_OBJECT_COW_1:
		n_frames = N_FRAMES_COW_1;
		for (int i = 0; i < n_frames; i++) {
			object_frames.emplace_back();
			strcpy(object_frames[i].filename, "Data/cow_vn.geom");
			object_frames[i].n_fields = 6;
			object_frames[i].front_face_mode = GL_CCW;
			object_frames[i].prepare_geom_of_static_object();
			object_frames[i].instances.emplace_back();
			cur_MM = &(object_frames[i].instances.back().ModelMatrix);
			*cur_MM = glm::scale(glm::mat4(1.0f), glm::vec3(30.0f, 30.0f, 30.0f));
			cur_material = &(object_frames[i].instances.back().material);
			cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			cur_material->ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
			cur_material->diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
			cur_material->specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
			cur_material->exponent = 128.0f * 0.21794872f;
		}
		break;
	case DYNAMIC_OBJECT_COW_2:
		n_frames = N_FRAMES_COW_2;
		for (int i = 0; i < n_frames; i++) {
			object_frames.emplace_back();
			strcpy(object_frames[i].filename, "Data/cow_vn.geom");
			object_frames[i].n_fields = 6;
			object_frames[i].front_face_mode = GL_CCW;
			object_frames[i].prepare_geom_of_static_object();

			object_frames[i].instances.emplace_back();
			cur_MM = &(object_frames[i].instances.back().ModelMatrix);
			*cur_MM = glm::scale(glm::mat4(1.0f), glm::vec3(30.0f, 30.0f, 30.0f));
			cur_material = &(object_frames[i].instances.back().material);
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			cur_material->ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
			cur_material->diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
			cur_material->specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
			cur_material->exponent = 128.0f * 0.6f;
		}
		break;
	}
}

// added from data files
void Nathan_D::define_object() {
#define N_NATHAN_FRAMES 69
	glm::mat4* cur_MM;
	Material* cur_material;
	flag_valid = true;

	for (int i = 0; i < N_NATHAN_FRAMES; i++) {
		object_frames.emplace_back();
		sprintf(object_frames[i].filename, "Data/dynamic_objects/nathan/rp_nathan_animated_003_walking%d.geom", i);
		object_frames[i].n_fields = 8;
		object_frames[i].front_face_mode = GL_CW;
		object_frames[i].prepare_geom_of_static_object();

		object_frames[i].instances.emplace_back();
		cur_MM = &(object_frames[i].instances.back().ModelMatrix);
		*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(-0.83f * i, 0.0f, 0.0f));		// to root model in place (why is it moving forward lmao)
		*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		*cur_MM = glm::scale(*cur_MM, glm::vec3(20.0f, 20.0f, 20.0f));
		cur_material = &(object_frames[i].instances.back().material);
		cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		cur_material->ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
		cur_material->diffuse = glm::vec4(0.69f, 0.46f, 0.94f, 1.0f);
		cur_material->specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
		cur_material->exponent = 128.0f * 0.21794872f;


	}
}

void Wolf_D::define_object() {
#define N_WOLF_FRAMES 17
	glm::mat4* cur_MM;
	Material* cur_material;
	flag_valid = true;

	for (int i = 0; i < N_WOLF_FRAMES; i++) {
		object_frames.emplace_back();
		sprintf(object_frames[i].filename, "Data/dynamic_objects/wolf/wolf_%d%d_vnt.geom", i / 10, i % 10);
		object_frames[i].n_fields = 8;
		object_frames[i].front_face_mode = GL_CW;
		object_frames[i].prepare_geom_of_static_object();

		object_frames[i].instances.emplace_back();
		cur_MM = &(object_frames[i].instances.back().ModelMatrix);
		*cur_MM = glm::rotate(glm::mat4(1.0f), 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		*cur_MM = glm::rotate(*cur_MM, 149.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));	// to line up with moving angle
		*cur_MM = glm::scale(*cur_MM, glm::vec3(30.0f, 30.0f, 30.0f));
		cur_material = &(object_frames[i].instances.back().material);
		cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		cur_material->ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
		cur_material->diffuse = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);	// light grey color
		cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // unused
		cur_material->exponent = 128.0f * 0.21794872f;
	}
}

void Dynamic_Object::draw_object(glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix, SHADER_ID shader_kind,
	std::vector<std::reference_wrapper<Shader>>& shader_list, int time_stamp, Light_Properties* lights, int n_lights) {
	int cur_object_index = time_stamp % object_frames.size();
	Static_Object& cur_object = object_frames[cur_object_index];
	glFrontFace(cur_object.front_face_mode);

	float rotation_angle = 0.0f;
	float nathan_walking_distance = 80.0f / 90;		// nathan walks back and forth between (80.0f, 100.0f, 0.0f) and (160.0f, 100.0f, 0.0f) within 3 seconds (180 frames)
	float wolf_walking_distance_x = 30.0f / 60;
	float wolf_walking_distance_y = 50.0f / 60;
	glm::mat4 ModelMatrix = glm::mat4(1.0f);
	switch (object_id) {
	case DYNAMIC_OBJECT_TIGER:
		rotation_angle = (time_stamp % 360) * TO_RADIAN;
		ModelMatrix = glm::rotate(ModelMatrix, -rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(100.0f, 0.0f, 0.0f));
		break;
	case DYNAMIC_OBJECT_COW_1:
		rotation_angle = (2 * time_stamp % 360) * TO_RADIAN;
		ModelMatrix = glm::rotate(ModelMatrix, -rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case DYNAMIC_OBJECT_COW_2:
		rotation_angle = (5 * time_stamp % 360) * TO_RADIAN;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(100.0f, 50.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, rotation_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case DYNAMIC_OBJECT_NATHAN:	// nathan walks back and forth within 3 seconds (180 frames)
		
		nathan_walking_distance *= (time_stamp % 90);	// halfway
		if (time_stamp % 360 < 90) {
			rotation_angle = 0.0f * TO_RADIAN;
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(80.0f + nathan_walking_distance, 100.0f, 0.0f));
		}
		else if (time_stamp % 360 < 180) {
			rotation_angle = 180.0f * TO_RADIAN;
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(160.0f - nathan_walking_distance, 100.0f, 0.0f));
		}
		else if (time_stamp % 360 < 270) {
			rotation_angle = -90.0f * TO_RADIAN;
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(80.0f, 100.0f - nathan_walking_distance, 0.0f));
		}
		else {
			rotation_angle = 90.0f * TO_RADIAN;
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(80.0f, 20.0f + nathan_walking_distance, 0.0f));
		}
		ModelMatrix = glm::rotate(ModelMatrix, rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case DYNAMIC_OBJECT_WOLF:	// wolf walks back and forth between (180.0f, 35.0f, 0.0f) and (210.0f, 85.0f, 0.0f)  within 2 seconds (120 frames) 
		
		wolf_walking_distance_x *= (time_stamp % 60);
		wolf_walking_distance_y *= (time_stamp % 60);

		glm::vec3 wolf_pos;
		if (time_stamp % 120 < 60) {
			rotation_angle = 0.0f * TO_RADIAN;
			wolf_pos = glm::vec3(180.0f + wolf_walking_distance_x, 35.0f + wolf_walking_distance_y, 0.0f);
			ModelMatrix = glm::translate(ModelMatrix, wolf_pos);
			
		}
		else {
			rotation_angle = 180.0f * TO_RADIAN;
			wolf_pos = glm::vec3(210.0f - wolf_walking_distance_x, 85.0f - wolf_walking_distance_y, 0.0f);
			ModelMatrix = glm::translate(ModelMatrix, wolf_pos);
			
		}
		ModelMatrix = glm::rotate(ModelMatrix, rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));

		// after calculating model matrix of wolf, adjust lights[1] to wolfs location
		//glm::mat4 wolf_world_matrix = ModelMatrix * cur_object.instances[0].ModelMatrix;

		// Define spotlight properties relative to the wolf model
		glm::vec3 light_offset(20.0f, 20.0f, 10.0f); // Positioned above and in front of the wolf
		glm::vec3 spot_direction_local(1.0f, 0.0f, 0.0f); // Pointing forward

		glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, 59.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMat = glm::rotate(rotationMat, rotation_angle, glm::vec3(0.0f, 0.0f, 1.0f));

		// Transform to World Coordinates
		glm::vec4 light_pos_world = ModelMatrix * glm::vec4(light_offset, 1.0f);
		glm::vec3 spot_dir_world = glm::vec3(rotationMat * glm::vec4(spot_direction_local, 0.0f));

		// Update the scene's light data (lights[1] is the wolf's spotlight)
		// We will transform to Eye Coords in the main draw loop before passing the uniform
		lights[1].position = light_pos_world;
		lights[1].spot_direction = glm::normalize(spot_dir_world);
		break;
	}

	for (int i = 0; i < cur_object.instances.size(); i++) {
		glm::mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix * cur_object.instances[i].ModelMatrix;
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix * cur_object.instances[i].ModelMatrix;
		glm::mat4 ModelViewMatrixInvTrans = glm::inverse(glm::transpose(ModelViewMatrix));
		switch (shader_kind) {
		case SHADER_SIMPLE: {
			Shader_Simple* shader_simple_ptr = static_cast<Shader_Simple*>(&shader_list[shader_ID_mapper[shader_kind]].get());
			glUseProgram(shader_simple_ptr->h_ShaderProgram);
			glUniformMatrix4fv(shader_simple_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE,
				&ModelViewProjectionMatrix[0][0]);
			glUniform3f(shader_simple_ptr->loc_primitive_color, cur_object.instances[i].material.diffuse.r,
				cur_object.instances[i].material.diffuse.g, cur_object.instances[i].material.diffuse.b);
			break;
			}
		case SHADER_PHONG: {
			Shader_Phong* shader_phong_ptr = static_cast<Shader_Phong*>(&shader_list[shader_ID_mapper[shader_kind]].get());
			glUseProgram(shader_phong_ptr->h_ShaderProgram);

			// Set matrices
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

			// Set material
			set_material(cur_object.instances[i].material, shader_phong_ptr->loc_material);

			// Set lights
			glUniform1i(shader_phong_ptr->loc_n_lights, n_lights);
			for (int j = 0; j < n_lights; j++) {
				glm::vec4 light_pos_EC = ViewMatrix * lights[j].position;
				glUniform1i(shader_phong_ptr->loc_lights[j].light_on, lights[j].light_on);
				glUniform4fv(shader_phong_ptr->loc_lights[j].position, 1, &light_pos_EC[0]);
				glUniform4fv(shader_phong_ptr->loc_lights[j].ambient_color, 1, &lights[j].ambient_color[0]);
				glUniform4fv(shader_phong_ptr->loc_lights[j].diffuse_color, 1, &lights[j].diffuse_color[0]);
				glUniform4fv(shader_phong_ptr->loc_lights[j].specular_color, 1, &lights[j].specular_color[0]);
				glUniform4fv(shader_phong_ptr->loc_lights[j].light_attenuation_factors, 1, &lights[j].light_attenuation_factors[0]);

				glm::vec3 spot_dir_EC = glm::mat3(ViewMatrix) * lights[j].spot_direction;
				glUniform3fv(shader_phong_ptr->loc_lights[j].spot_direction, 1, &spot_dir_EC[0]);
				glUniform1f(shader_phong_ptr->loc_lights[j].spot_exponent, lights[j].spot_exponent);
				glUniform1f(shader_phong_ptr->loc_lights[j].spot_cutoff_angle, lights[j].spot_cutoff_angle);
			}
			break;
		}
		case SHADER_GOURAUD: {
			Shader_Gouraud* shader_gouraud_ptr = static_cast<Shader_Gouraud*>(&shader_list[shader_ID_mapper[shader_kind]].get());
			glUseProgram(shader_gouraud_ptr->h_ShaderProgram);

			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

			set_material(cur_object.instances[i].material, shader_gouraud_ptr->loc_material);

			glUniform1i(shader_gouraud_ptr->loc_n_lights, n_lights);
			for (int j = 0; j < n_lights; j++) {
				glUniform1i(shader_gouraud_ptr->loc_lights[j].light_on, lights[j].light_on);
				glm::vec4 light_pos_EC = ViewMatrix * lights[j].position;
				glUniform4fv(shader_gouraud_ptr->loc_lights[j].position, 1, &light_pos_EC[0]);
				glUniform4fv(shader_gouraud_ptr->loc_lights[j].ambient_color, 1, &lights[j].ambient_color[0]);
				glUniform4fv(shader_gouraud_ptr->loc_lights[j].diffuse_color, 1, &lights[j].diffuse_color[0]);
				glUniform4fv(shader_gouraud_ptr->loc_lights[j].specular_color, 1, &lights[j].specular_color[0]);
				glUniform4fv(shader_gouraud_ptr->loc_lights[j].light_attenuation_factors, 1, &lights[j].light_attenuation_factors[0]);
				glm::vec3 spot_dir_EC = glm::mat3(ViewMatrix) * lights[j].spot_direction;
				glUniform3fv(shader_gouraud_ptr->loc_lights[j].spot_direction, 1, &spot_dir_EC[0]);
				glUniform1f(shader_gouraud_ptr->loc_lights[j].spot_exponent, lights[j].spot_exponent);
				glUniform1f(shader_gouraud_ptr->loc_lights[j].spot_cutoff_angle, lights[j].spot_cutoff_angle);
			}
			break;
		}
		}
		glBindVertexArray(cur_object.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3 * cur_object.n_triangles);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}