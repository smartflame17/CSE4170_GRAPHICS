#define _CRT_SECURE_NO_WARNINGS

#include "Scene_Definitions.h"

void Static_Object::read_geometry(int bytes_per_primitive) {
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open the object file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	vertices = (float*)malloc(n_triangles * bytes_per_primitive);
	if (vertices == NULL) {
		fprintf(stderr, "Error: cannot allocate memory for the geometry file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(vertices, bytes_per_primitive, n_triangles, fp); // assume the data file has no faults.
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);
}

void Static_Object::prepare_geom_of_static_object() {
	int i, n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = n_fields * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	read_geometry(n_bytes_per_triangle);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, n_triangles * n_bytes_per_triangle, vertices, GL_STATIC_DRAW);

	free(vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	// added for objects with texture and normals
	if (n_fields >= 6) { // Normals are present (V N or V N T)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	if (n_fields >= 8) { // Texture coordinates are present (V N T)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Building::define_object() {
	Material* cur_material;
	strcpy(filename, "Data/Building1_vnt.geom");
	n_fields = 8;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	instances.back().ModelMatrix = glm::mat4(1.0f);

	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	cur_material->diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); //glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	cur_material->exponent = 128.0f * 0.1f;
}

void Table::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;

	strcpy(filename, "Data/Table_vn.geom");
	n_fields = 6;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(157.0f, 76.5f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(0.5f, 0.5f, 0.5f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.1f, 0.3f, 0.1f, 1.0f);
	cur_material->diffuse = glm::vec4(0.4f, 0.6f, 0.3f, 1.0f);
	cur_material->specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	cur_material->exponent = 15.0f;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(198.0f, 120.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(0.8f, 0.6f, 0.6f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	cur_material->diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	cur_material->specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	cur_material->exponent = 128.0f * 0.078125f;
}

void Light::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/Light_vn.geom");
	n_fields = 6;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 100.0f, 49.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	cur_material->diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	cur_material->specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	cur_material->exponent = 128.0f * 0.4f;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 47.5f, 49.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	cur_material->diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	cur_material->specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	cur_material->exponent = 128.0f * 0.4f;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 130.0f, 49.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	cur_material->diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	cur_material->specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	cur_material->exponent = 128.0f * 0.4f;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, 60.0f, 49.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	cur_material->diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	cur_material->specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	cur_material->exponent = 128.0f * 0.4f;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 112.5f, 49.0));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	cur_material->diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	cur_material->specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	cur_material->exponent = 128.0f * 0.4f;

}

void Teapot::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/Teapotn_vn.geom");
	n_fields = 6;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(2.0f, 2.0f, 2.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	cur_material->diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	cur_material->specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	cur_material->exponent = 128.0f * 0.6;
}

void New_Chair::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/new_chair_vnt.geom");
	n_fields = 8;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 104.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(0.8f, 0.8f, 0.8f));
	*cur_MM = glm::rotate(*cur_MM, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	cur_material->diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	cur_material->specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	cur_material->exponent = 128.0f * 0.078125f;
}

void Frame::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/Frame_vn.geom");
	n_fields = 6;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 116.0f, 30.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(0.6f, 0.6f, 0.6f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	cur_material->diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	cur_material->specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	cur_material->exponent = 128.0f * 0.1f;
}

void New_Picture::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/new_picture_vnt.geom");
	n_fields = 8;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(189.5f, 116.0f, 30.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(13.5f * 0.6f, 13.5f * 0.6f, 13.5f * 0.6f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	cur_material->diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	cur_material->specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	cur_material->exponent = 128.0f * 0.6f;
}

void Cow::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/cow_vn.geom");
	n_fields = 6;
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(215.0f, 100.0f, 9.5f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(30.0f, 30.0f, 30.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	cur_material = &(instances.back().material);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	cur_material->diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	cur_material->specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	cur_material->exponent = 0.21794872f * 0.6f;
}

// added from data files
void Bike::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/static_objects/bike_vnt.geom");
	n_fields = 8;	// IMPORTANT TO SET AS 8 IF CONTAINING TEXCOORDS!!
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 120.0f, 0.0f));		// at top left outer wall of building
	*cur_MM = glm::rotate(*cur_MM, 5.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));	// tilted slightly against wall
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(20.0f, 20.0f, 20.0f));

	cur_material = &(instances.back().material);
	cur_material->diffuse = glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Example
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Not used by simple shader
	cur_material->exponent = 0.0f; // Not used by simple shader
}

void Cat::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/static_objects/cat_vnt.geom");
	n_fields = 8;	// IMPORTANT TO SET AS 8 IF CONTAINING TEXCOORDS!!
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(107.0f, 30.0f, 0.0f));		// at bottom right outer wall of building
	*cur_MM = glm::rotate(*cur_MM, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));	// face +y direction
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(15.0f, 15.0f, 15.0f));

	cur_material = &(instances.back().material);
	cur_material->diffuse = glm::vec4(1.0f, 0.8f, 0.25f, 1.0f);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Example
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Not used by simple shader
	cur_material->exponent = 0.0f; // Not used by simple shader
}

void Tank::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/static_objects/tank_vnt.geom");
	n_fields = 8;	// IMPORTANT TO SET AS 8 IF CONTAINING TEXCOORDS!!
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 110.0f, 0.0f));		// at top right room of building
	//*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(2.5f, 2.5f, 2.5f));

	cur_material = &(instances.back().material);
	cur_material->diffuse = glm::vec4(0.27f, 0.54f, 0.2f, 1.0f);	// dark green color
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Example
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Not used by simple shader
	cur_material->exponent = 0.0f; // Not used by simple shader
}

void Helicopter::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/static_objects/helicopter_vntm.geom");
	n_fields = 8;	// IMPORTANT TO SET AS 8 IF CONTAINING TEXCOORDS!!
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 40.0f, 0.0f));		// at bottom left room of building
	//*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(1.5f, 1.5f, 1.5f));

	cur_material = &(instances.back().material);
	cur_material->diffuse = glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Example
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Not used by simple shader
	cur_material->exponent = 0.0f; // Not used by simple shader
}

void Godzilla::define_object() {
	glm::mat4* cur_MM;
	Material* cur_material;
	strcpy(filename, "Data/static_objects/godzilla_vnt.geom");
	n_fields = 8;	// IMPORTANT TO SET AS 8 IF CONTAINING TEXCOORDS!!
	front_face_mode = GL_CCW;
	prepare_geom_of_static_object();
	flag_valid = true;

	instances.emplace_back();
	cur_MM = &(instances.back().ModelMatrix);
	*cur_MM = glm::translate(glm::mat4(1.0f), glm::vec3(110.0f, 140.0f, 0.0f));		// at bottom middle room of building
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));	// facing +x direction
	*cur_MM = glm::rotate(*cur_MM, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	*cur_MM = glm::scale(*cur_MM, glm::vec3(0.1f, 0.1f, 0.1f));

	cur_material = &(instances.back().material);
	cur_material->diffuse = glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);
	cur_material->emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	cur_material->ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Example
	cur_material->specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Not used by simple shader
	cur_material->exponent = 0.0f; // Not used by simple shader
}

void print_mat4(const char* string, glm::mat4 M) {
	fprintf(stdout, "\n***** %s ******\n", string);
	for (int i = 0; i < 4; i++)
		fprintf(stdout, "*** COL[%d] (%f, %f, %f, %f)\n", i, M[i].x, M[i].y, M[i].z, M[i].w);
	fprintf(stdout, "**************\n\n");
}


void Static_Object::draw_object(glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix, SHADER_ID shader_kind,
	std::vector<std::reference_wrapper<Shader>>& shader_list, Light_Properties* lights, int n_lights) {
	glm::mat4 ModelViewProjectionMatrix, ModelViewMatrix, ModelViewMatrixInvTrans;
	glFrontFace(front_face_mode);
	for (int i = 0; i < instances.size(); i++) {
		ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * instances[i].ModelMatrix;
		ModelViewMatrix = ViewMatrix * instances[i].ModelMatrix;
		switch (shader_kind) {
		case SHADER_SIMPLE: {
			Shader_Simple* shader_simple_ptr = static_cast<Shader_Simple*>(&shader_list[shader_ID_mapper[shader_kind]].get());
			glUseProgram(shader_simple_ptr->h_ShaderProgram);
			glUniformMatrix4fv(shader_simple_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE,
				&ModelViewProjectionMatrix[0][0]);
			glUniform3f(shader_simple_ptr->loc_primitive_color, instances[i].material.diffuse.r,
				instances[i].material.diffuse.g, instances[i].material.diffuse.b);

			break;
		}
		case SHADER_PHONG: {
			Shader_Phong* shader_phong_ptr = static_cast<Shader_Phong*>(&shader_list[shader_ID_mapper[shader_kind]].get());
			glUseProgram(shader_phong_ptr->h_ShaderProgram);

			// Set matrices
			ModelViewMatrixInvTrans = glm::inverse(glm::transpose(ModelViewMatrix));
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
			glUniformMatrix4fv(shader_phong_ptr->loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

			// Set material
			set_material(instances[i].material, shader_phong_ptr->loc_material);

			// Set lights
			glUniform1i(shader_phong_ptr->loc_n_lights, n_lights);
			for (int j = 0; j < n_lights; j++) {
				// Convert world-space light position to eye-space and set uniform
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

			ModelViewMatrixInvTrans = glm::inverse(glm::transpose(ModelViewMatrix));
			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
			glUniformMatrix4fv(shader_gouraud_ptr->loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

			set_material(instances[i].material, shader_gouraud_ptr->loc_material);

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
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3 * n_triangles);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}
