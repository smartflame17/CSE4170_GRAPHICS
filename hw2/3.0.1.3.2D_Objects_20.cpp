#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

														  // include glm/*.hpp only if necessary
														  //#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.

glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define PI 3.14159265358979323846
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 1200, win_height = 900;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;

int start_time = 0;		// for timer function
int prev_mouse_x = 0;
int prev_mouse_y = 0;	// for mouse position

// --- Game state related ---
#define STANDARD_MODE 0
#define DEBUG_MODE 1
int game_mode = STANDARD_MODE;
int object_count = 0;

// --- Skybox related ---
const float skybox_y = 50.0f;		// The border between 'ground' and 'sky'

// --- Airplane related ---
// airplane position
float airplane_x = 0.1f;
float airplane_y = 0.0f;
float airplane_angle = 0.0f;
const int airplane_animation_duration = 5000;  // 2000ms animation duration

// trajectory of airplane (sine wave)
const float sine_amplitude = 60.0f;
const float sine_frequency = 2.0f;
float initial_airplane_x = 0.1f; // Store initial state for reset
float initial_airplane_y = 0.0f;
float initial_airplane_angle = 0.0f;

struct AABB {
	float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
};

// --- Ryu related ---
#define RYU_IDLE 0
#define RYU_FIRE 1

float ryu_pos_x = -400.0f;
float ryu_pos_y = -200.0f;
float ryu_scale = 4.0f; // Store Ryu's scale factor
float initial_ryu_pos_x = -400.0f; // Store initial state for reset
float initial_ryu_pos_y = -200.0f;
const float ryu_max_health = 100.0f;
float ryu_current_health = ryu_max_health;
bool is_dragging_ryu = false;
int ryu_state = RYU_IDLE;
AABB ryu_aabb;

// --- Hadouken related ---
int last_hadouken_time = 0;             // Time the last Hadouken was fired
const int HADOUKEN_COOLDOWN_MS = 500;   // Cooldown in milliseconds (0.5 seconds)
const float HADOUKEN_SPEED = 8.0f;      // Speed of the Hadouken projectile
const float HADOUKEN_START_OFFSET_X = 20.0f * ryu_scale; // Offset from Ryu's center (scaled)
const float HADOUKEN_START_OFFSET_Y = 5.0f * ryu_scale; // Vertical offset from Ryu's center (scaled)
const float HADOUKEN_SCALE = 1.0f;      // Scale factor for drawing Hadouken

struct Hadouken {
	glm::vec2 position;
	float current_scale = HADOUKEN_SCALE;
	bool active = true;
	AABB world_aabb;	// Updated collision box each frame
};
std::vector<Hadouken> active_hadoukens; // Vector to store active Hadoukens

// --- Cocktail related ---
const float cocktail_base_scale = 2.0f;
const float cocktail_pulsate_amplitude = 0.25f;
const float cocktail_pulsate_period_ms = 2000.0f;
float current_cocktail_scale = cocktail_base_scale;

struct CocktailInstance {
	glm::vec2 position;
	float current_scale = cocktail_base_scale;
	bool active = true;
	AABB world_aabb;
};
const float cocktail_speed = -0.5f;
std::vector<CocktailInstance> active_cocktails;

// --- Car2 related ---
struct Car2Instance {
	glm::vec2 relative_position; // Position relative to parent's origin after parent rotation
	float scale = 1.0f;          // Scale relative to parent's scale
	float rotation_angle = 0.0f; // Base rotation relative to parent
	float parent_rotation_offset = 0.0f;
	bool active = true;
	AABB world_aabb;             // Calculated world AABB
};
const float car2RotationOffset = 30 / 60.0f;	// Degrees per second
const float car2_relative_dist = 80.0f;			// Distance from parent carInstance

// --- Car related ---
struct CarInstance {
	glm::vec2 position;
	float scale = 1.0f;
	bool active = true;
	AABB world_aabb;
	std::vector<Car2Instance> child_car2s; // Embedded child Car2s
};
float car_speed = -1.0f;			// speed of parent car moving (negative means moving left)
std::vector<CarInstance> active_cars;
float car_scale = 2.0f;
float car2_scale = 1.0f; // Relative scale to car

// --- Collision boxes ---
const AABB RYU_AABB = { -7.0f, -7.0f, 9.0f, 16.0f };
const AABB HADOUKEN_AABB = { -30.0f, -20.0f, 20.0f, 20.0f };
const AABB CAR_AABB = { -16.0f, -12.0f, 16.0f, 10.0f };
const AABB CAR2_AABB = { -18.0f, -11.0f, 18.0f, 8.0f };
const AABB COCKTAIL_AABB = { -12.0f, -12.0f, 16.0f, 18.0f };

// --- For Debugging AABB ---
GLuint VBO_aabb_box, VAO_aabb_box;
const GLfloat AABB_BOX_COLOR[3] = { 1.0f, 0.0f, 0.0f }; // Red color for AABB

void prepare_aabb_box() {
	// Simple rectangle vertices (will be scaled/translated in draw call)
	GLfloat vertices[][2] = {
		{ -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f }
	};

	glGenBuffers(1, &VBO_aabb_box);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_aabb_box);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO_aabb_box);
	glBindVertexArray(VAO_aabb_box);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_aabb_box);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(LOC_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_aabb_box(const AABB& aabb) {
	// Calculate center and dimensions of the AABB
	float center_x = (aabb.minX + aabb.maxX) / 2.0f;
	float center_y = (aabb.minY + aabb.maxY) / 2.0f;
	float scale_x = aabb.maxX - aabb.minX;
	float scale_y = aabb.maxY - aabb.minY;

	// Create Model matrix to translate and scale the unit box
	glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(center_x, center_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scale_x, scale_y, 1.0f));

	// Set MVP matrix and color for the AABB box
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniform3fv(loc_primitive_color, 1, AABB_BOX_COLOR);

	// Bind the AABB VAO and draw the outline
	glBindVertexArray(VAO_aabb_box);
	glDrawArrays(GL_LINE_LOOP, 0, 4); // Use GL_LINE_LOOP for outline
	glBindVertexArray(0);
}


GLfloat axes[4][2];
GLfloat axes_color[3] = { 0.0f, 0.0f, 0.0f };
GLuint VBO_axes, VAO_axes;

void prepare_axes(void) { // Draw axes in their MC.
	axes[0][0] = -win_width / 2.5f; axes[0][1] = 0.0f;
	axes[1][0] = win_width / 2.5f; axes[1][1] = 0.0f;
	axes[2][0] = 0.0f; axes[2][1] = -win_height / 2.5f;
	axes[3][0] = 0.0f; axes[3][1] = win_height / 2.5f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_axes(void) {
	axes[0][0] = -win_width / 2.25f; axes[1][0] = win_width / 2.25f;
	axes[2][1] = -win_height / 2.25f;
	axes[3][1] = win_height / 2.25f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_axes(void) {
	glUniform3fv(loc_primitive_color, 1, axes_color);
	glBindVertexArray(VAO_axes);
	glDrawArrays(GL_LINES, 0, 4);
	glBindVertexArray(0);
}

GLfloat line[2][2];
GLfloat line_color[3] = { 1.0f, 0.0f, 0.0f };
GLuint VBO_line, VAO_line;

void prepare_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_line);
	glBindVertexArray(VAO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_line(void) { // Draw line in its MC.
					   // y = x - win_height/4
	glUniform3fv(loc_primitive_color, 1, line_color);
	glBindVertexArray(VAO_line);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

GLfloat skybox[4][2] = { {-600.0, 450.0}, {-600.0, skybox_y}, {600.0, skybox_y}, {600.0, 450.0} };
GLfloat skybox_color[3] = { 206 / 255.0f, 242 / 255.0f, 239 / 255.0f };
GLuint VBO_skybox, VAO_skybox;

void prepare_skybox() {
	GLsizeiptr buffer_size = sizeof(skybox);

	glGenBuffers(1, &VBO_skybox);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_skybox);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skybox), skybox);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_skybox);
	glBindVertexArray(VAO_skybox);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_skybox);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_skybox() {
	glBindVertexArray(VAO_skybox);
	glUniform3fv(loc_primitive_color, 1, skybox_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

// healthbar
GLfloat healthbar[4][2] = { {100.0, 5.0}, {0.0, 5.0}, {0.0, -5.0}, {100.0, -5.0} };		// 100 x 10 size
GLfloat healthbar_color[3] = { 255 / 255.0f, 255 / 255.0f, 0 / 255.0f };		// yellow
GLfloat healthbar_red_color[3] = { 255 / 255.0f, 0 / 255.0f, 0 / 255.0f };		// red
GLfloat healthbar_border_color[3] = { 240 / 255.0f, 240 / 255.0f, 249 / 255.0f };	// white
GLuint VBO_healthbar, VAO_healthbar;

void prepare_healthbar() {
	GLsizeiptr buffer_size = sizeof(healthbar);

	glGenBuffers(1, &VBO_healthbar);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_healthbar);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(healthbar), healthbar);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_healthbar);
	glBindVertexArray(VAO_healthbar);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_healthbar);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_healthbar() {
	glBindVertexArray(VAO_healthbar);
	glUniform3fv(loc_primitive_color, 1, healthbar_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void draw_healthbar_red() {
	glBindVertexArray(VAO_healthbar);
	glUniform3fv(loc_primitive_color, 1, healthbar_red_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void draw_healthbar_border() {
	glBindVertexArray(VAO_healthbar);
	glUniform3fv(loc_primitive_color, 1, healthbar_border_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

// ryu
#define RYU_HEAD 0
#define RYU_HAIR 1
#define RYU_HEADBAND 2
#define RYU_BODY 3
#define RYU_LEFTARM 4
#define RYU_RIGHTARM 5
#define RYU_LEFTLEG 6
#define RYU_RIGHTLEG 7
#define RYU_WAISTBAND 8
GLfloat ryu_head[4][2] = { {3.0, 10.0}, {3.0, 16.0}, {-3.0, 16.0}, {-3.0, 10.0} };
GLfloat ryu_hair[4][2] = { {3.0, 14.0}, {3.0, 16.0}, {-3.0, 16.0}, {-3.0, 14.0} };
GLfloat ryu_headband[4][2] = { {3.0, 13.0}, {3.0, 14.0}, {-3.0, 14.0}, {-3.0, 13.0} };
GLfloat ryu_body[8][2] = { {7.0, 10.0}, {-7.0, 10.0}, {-7.0, 3.0}, {-5.0, 3.0},  { -5.0, 0.0 }, {5.0, 0.0}, {5.0, 3.0}, {7.0, 3.0} };
GLfloat ryu_leftarm[4][2] = { {7.0, 10.0}, {5.0, 9.0}, {7.0, 3.0}, {9.0, 4.0} };
GLfloat ryu_rightarm[6][2] = { {-3.0, 10.0}, {-5.0, 9.0}, {-3.0, 5.0}, {2.0, 7.0}, {1.0, 9.0}, {-2.0, 8.0} };
GLfloat ryu_leftleg[4][2] = { {5.0, 2.0}, {2.0, 0.0}, {4.0, -7.0}, {7.0, -5.0} };
GLfloat ryu_rightleg[6][2] = { {-2.0, 0.0},  { -5.0, 1.0 }, {-6.0, -4.0}, {-4.0, -7.0}, {-1.0, -6.0}, {-3.0, -3.0} };
GLfloat ryu_waistband[4][2] = { {6.0, 3.0}, {-4.0, 3.0}, {-4.0, 2.0}, {6.0, 2.0} };
GLfloat ryu_color[9][3] = {
	{ 255 / 255.0f, 227 / 255.0f, 186 / 255.0f }, //head
	{ 61 / 255.0f, 52 / 255.0f, 39 / 255.0f }, //hair
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },	//headband
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f }, //body
	{ 255 / 255.0f, 227 / 255.0f, 186 / 255.0f }, //leftarm
	{ 255 / 255.0f, 227 / 255.0f, 186 / 255.0f }, //rightarm
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f }, //leftleg
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f }, //rightleg
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }  //waistband
};

GLuint VBO_ryu, VAO_ryu;
void prepare_ryu() {
	GLsizeiptr buffer_size = sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body) 
		+ sizeof(ryu_leftarm) + sizeof(ryu_rightarm) + sizeof(ryu_leftleg) + sizeof(ryu_rightleg) + sizeof(ryu_waistband);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_ryu);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ryu);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ryu_head), ryu_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head), sizeof(ryu_hair), ryu_hair);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair), sizeof(ryu_headband), ryu_headband);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband), sizeof(ryu_body), ryu_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body), sizeof(ryu_leftarm), ryu_leftarm);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body) + sizeof(ryu_leftarm),
		sizeof(ryu_rightarm), ryu_rightarm);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body) + sizeof(ryu_leftarm)
		+ sizeof(ryu_rightarm), sizeof(ryu_leftleg), ryu_leftleg);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body) + sizeof(ryu_leftarm)
		+ sizeof(ryu_rightarm) + sizeof(ryu_leftleg), sizeof(ryu_rightleg), ryu_rightleg);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body) + sizeof(ryu_leftarm)
		+ sizeof(ryu_rightarm) + sizeof(ryu_leftleg) + sizeof(ryu_rightleg), sizeof(ryu_waistband), ryu_waistband);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_ryu);
	glBindVertexArray(VAO_ryu);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ryu);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_ryu() {
	glBindVertexArray(VAO_ryu);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HAIR]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HEADBAND]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 8);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_LEFTARM]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_RIGHTARM]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 6);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_LEFTLEG]);
	glDrawArrays(GL_TRIANGLE_FAN, 30, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_RIGHTLEG]);
	glDrawArrays(GL_TRIANGLE_FAN, 34, 6);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_WAISTBAND]);
	glDrawArrays(GL_TRIANGLE_FAN, 40, 4);

	glBindVertexArray(0);
}

// separate arm, leg position for ryu_fire model
GLfloat ryu_body_fire[8][2] = { {7.0, 10.0}, {-3.0, 10.0}, {-4.0, 3.0}, {-4.0, 2.0},  { -5.0, 0.0 }, {5.0, 0.0}, {5.0, 3.0}, {7.0, 3.0} };
GLfloat ryu_leftarm_fire[5][2] = { {7.0, 10.0}, {7.0, 8.0}, {14.0, 8.0}, {14.0, 11.0}, {13.0, 10.0} };
GLfloat ryu_rightarm_fire[7][2] = { {4.0, 9.0}, {3.0, 7.0}, {7.0, 5.0}, {13.0, 5.0}, {14.0, 4.0}, {14.0, 7.0}, {7.0, 7.0} };
GLfloat ryu_leftleg_fire[6][2] = { {5.0, 2.0}, {2.0, 0.0}, {3.0, -3.0}, {2.0, -7.0}, {5.0, -7.0}, {7.0, -2.0} };
GLfloat ryu_rightleg_fire[4][2] = { {-2.0, 0.0},  { -5.0, 1.0 }, {-10.0, -7.0}, {-7.0, -7.0} };

GLuint VBO_ryu_fire, VAO_ryu_fire;

void prepare_ryu_fire() {
	GLsizeiptr buffer_size = sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire)
		+ sizeof(ryu_leftarm_fire) + sizeof(ryu_rightarm_fire) + sizeof(ryu_leftleg_fire) + sizeof(ryu_rightleg_fire) + sizeof(ryu_waistband);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_ryu_fire);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ryu_fire);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ryu_head), ryu_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head), sizeof(ryu_hair), ryu_hair);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair), sizeof(ryu_headband), ryu_headband);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband), sizeof(ryu_body_fire), ryu_body_fire);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire), sizeof(ryu_leftarm_fire), ryu_leftarm_fire);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire) + sizeof(ryu_leftarm_fire),
		sizeof(ryu_rightarm_fire), ryu_rightarm_fire);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire) + sizeof(ryu_leftarm_fire)
		+ sizeof(ryu_rightarm_fire), sizeof(ryu_leftleg_fire), ryu_leftleg_fire);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire) + sizeof(ryu_leftarm_fire)
		+ sizeof(ryu_rightarm_fire) + sizeof(ryu_leftleg_fire), sizeof(ryu_rightleg_fire), ryu_rightleg_fire);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ryu_head) + sizeof(ryu_hair) + sizeof(ryu_headband) + sizeof(ryu_body_fire) + sizeof(ryu_leftarm_fire)
		+ sizeof(ryu_rightarm_fire) + sizeof(ryu_leftleg_fire) + sizeof(ryu_rightleg_fire), sizeof(ryu_waistband), ryu_waistband);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_ryu_fire);
	glBindVertexArray(VAO_ryu_fire);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ryu_fire);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_ryu_fire() {
	glBindVertexArray(VAO_ryu_fire);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HAIR]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_HEADBAND]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 8);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_LEFTARM]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 5);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_RIGHTARM]);
	glDrawArrays(GL_TRIANGLE_FAN, 25, 7);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_LEFTLEG]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 6);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_RIGHTLEG]);
	glDrawArrays(GL_TRIANGLE_FAN, 38, 4);

	glUniform3fv(loc_primitive_color, 1, ryu_color[RYU_WAISTBAND]);
	glDrawArrays(GL_TRIANGLE_FAN, 42, 4);

	glBindVertexArray(0);
}

// hadouken
GLfloat hadouken[9][2] = { {10.0, 20.0}, {-30.0, 20.0}, {-20.0, 10.0}, {-30.0, 0.0}, {-20.0, -10.0}, {-30.0, -20.0}, {10.0, -20.0}, {20.0, -10.0}, {20.0, 10.0} };
GLfloat hadouken_color[3] = {67 / 255.0f, 204 / 255.0f, 222 / 255.0f};

GLuint VBO_hadouken, VAO_hadouken;

void prepare_hadouken() {
	GLsizeiptr buffer_size = sizeof(hadouken);

	glGenBuffers(1, &VBO_hadouken);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hadouken);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hadouken), hadouken);
	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_hadouken);
	glBindVertexArray(VAO_hadouken);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hadouken);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_hadouken() {
	glBindVertexArray(VAO_hadouken);
	glUniform3fv(loc_primitive_color, 1, hadouken_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 9);
	glBindVertexArray(0);
}

// airplane
#define AIRPLANE_BIG_WING 0
#define AIRPLANE_SMALL_WING 1
#define AIRPLANE_BODY 2
#define AIRPLANE_BACK 3
#define AIRPLANE_SIDEWINDER1 4
#define AIRPLANE_SIDEWINDER2 5
#define AIRPLANE_CENTER 6
GLfloat big_wing[6][2] = { { 0.0, 0.0 },{ -20.0, 15.0 },{ -20.0, 20.0 },{ 0.0, 23.0 },{ 20.0, 20.0 },{ 20.0, 15.0 } };
GLfloat small_wing[6][2] = { { 0.0, -18.0 },{ -11.0, -12.0 },{ -12.0, -7.0 },{ 0.0, -10.0 },{ 12.0, -7.0 },{ 11.0, -12.0 } };
GLfloat body[5][2] = { { 0.0, -25.0 },{ -6.0, 0.0 },{ -6.0, 22.0 },{ 6.0, 22.0 },{ 6.0, 0.0 } };
GLfloat back[5][2] = { { 0.0, 25.0 },{ -7.0, 24.0 },{ -7.0, 21.0 },{ 7.0, 21.0 },{ 7.0, 24.0 } };
GLfloat sidewinder1[5][2] = { { -20.0, 10.0 },{ -18.0, 3.0 },{ -16.0, 10.0 },{ -18.0, 20.0 },{ -20.0, 20.0 } };
GLfloat sidewinder2[5][2] = { { 20.0, 10.0 },{ 18.0, 3.0 },{ 16.0, 10.0 },{ 18.0, 20.0 },{ 20.0, 20.0 } };
GLfloat center[1][2] = { { 0.0, 0.0 } };
GLfloat airplane_color[7][3] = {
	{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // big_wing
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // small_wing
	{ 111 / 255.0f,  85 / 255.0f, 157 / 255.0f },  // body
	{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // back
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder1
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder2
	{ 255 / 255.0f,   0 / 255.0f,   0 / 255.0f }   // center
};

GLuint VBO_airplane, VAO_airplane;

int airplane_clock = 0;
float airplane_s_factor = 1.0f;

void prepare_airplane() {
	GLsizeiptr buffer_size = sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2) + sizeof(center);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(big_wing), big_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing), sizeof(small_wing), small_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing), sizeof(body), body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body), sizeof(back), back);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back),
		sizeof(sidewinder1), sidewinder1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1), sizeof(sidewinder2), sidewinder2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2), sizeof(center), center);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane);
	glBindVertexArray(VAO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_airplane() { // Draw airplane in its MC.
	glBindVertexArray(VAO_airplane);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BIG_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SMALL_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BACK]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER1]);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER2]);
	glDrawArrays(GL_TRIANGLE_FAN, 27, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 32, 1);
	glPointSize(1.0);
	glBindVertexArray(0);
}

//shirt
#define SHIRT_LEFT_BODY 0
#define SHIRT_RIGHT_BODY 1
#define SHIRT_LEFT_COLLAR 2
#define SHIRT_RIGHT_COLLAR 3
#define SHIRT_FRONT_POCKET 4
#define SHIRT_BUTTON1 5
#define SHIRT_BUTTON2 6
#define SHIRT_BUTTON3 7
#define SHIRT_BUTTON4 8
GLfloat left_body[6][2] = { { 0.0, -9.0 },{ -8.0, -9.0 },{ -11.0, 8.0 },{ -6.0, 10.0 },{ -3.0, 7.0 },{ 0.0, 9.0 } };
GLfloat right_body[6][2] = { { 0.0, -9.0 },{ 0.0, 9.0 },{ 3.0, 7.0 },{ 6.0, 10.0 },{ 11.0, 8.0 },{ 8.0, -9.0 } };
GLfloat left_collar[4][2] = { { 0.0, 9.0 },{ -3.0, 7.0 },{ -6.0, 10.0 },{ -4.0, 11.0 } };
GLfloat right_collar[4][2] = { { 0.0, 9.0 },{ 4.0, 11.0 },{ 6.0, 10.0 },{ 3.0, 7.0 } };
GLfloat front_pocket[6][2] = { { 5.0, 0.0 },{ 4.0, 1.0 },{ 4.0, 3.0 },{ 7.0, 3.0 },{ 7.0, 1.0 },{ 6.0, 0.0 } };
GLfloat button1[3][2] = { { -1.0, 6.0 },{ 1.0, 6.0 },{ 0.0, 5.0 } };
GLfloat button2[3][2] = { { -1.0, 3.0 },{ 1.0, 3.0 },{ 0.0, 2.0 } };
GLfloat button3[3][2] = { { -1.0, 0.0 },{ 1.0, 0.0 },{ 0.0, -1.0 } };
GLfloat button4[3][2] = { { -1.0, -3.0 },{ 1.0, -3.0 },{ 0.0, -4.0 } };

GLfloat shirt_color[9][3] = {
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f },
	{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f },
	{ 206 / 255.0f, 173 / 255.0f, 184 / 255.0f }
};

GLuint VBO_shirt, VAO_shirt;
void prepare_shirt() {
	GLsizeiptr buffer_size = sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2) + sizeof(button3) + sizeof(button4);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_shirt);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_shirt);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(left_body), left_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body), sizeof(right_body), right_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body), sizeof(left_collar), left_collar);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar), sizeof(right_collar), right_collar);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar),
		sizeof(front_pocket), front_pocket);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket), sizeof(button1), button1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1), sizeof(button2), button2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2), sizeof(button3), button3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(left_body) + sizeof(right_body) + sizeof(left_collar) + sizeof(right_collar)
		+ sizeof(front_pocket) + sizeof(button1) + sizeof(button2) + sizeof(button3), sizeof(button4), button4);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_shirt);
	glBindVertexArray(VAO_shirt);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_shirt);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_shirt() {
	glBindVertexArray(VAO_shirt);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_LEFT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_RIGHT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_LEFT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_RIGHT_COLLAR]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_FRONT_POCKET]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 6);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON1]);
	glDrawArrays(GL_TRIANGLE_FAN, 26, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON2]);
	glDrawArrays(GL_TRIANGLE_FAN, 29, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON3]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, shirt_color[SHIRT_BUTTON4]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);
	glBindVertexArray(0);
}

//house
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = { { -12.0, 0.0 },{ 0.0, 12.0 },{ 12.0, 0.0 } };
GLfloat house_body[4][2] = { { -12.0, -14.0 },{ -12.0, 0.0 },{ 12.0, 0.0 },{ 12.0, -14.0 } };
GLfloat chimney[4][2] = { { 6.0, 6.0 },{ 6.0, 14.0 },{ 10.0, 14.0 },{ 10.0, 2.0 } };
GLfloat door[4][2] = { { -8.0, -14.0 },{ -8.0, -8.0 },{ -4.0, -8.0 },{ -4.0, -14.0 } };
GLfloat window[4][2] = { { 4.0, -6.0 },{ 4.0, -2.0 },{ 8.0, -2.0 },{ 8.0, -6.0 } };

GLfloat house_color[5][3] = {
	{ 200 / 255.0f, 39 / 255.0f, 42 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
	{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLuint VBO_house, VAO_house;
void prepare_house() {
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door)
		+ sizeof(window);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
		sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_house() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}

//car
#define CAR_BODY 0
#define CAR_FRAME 1
#define CAR_WINDOW 2
#define CAR_LEFT_LIGHT 3
#define CAR_RIGHT_LIGHT 4
#define CAR_LEFT_WHEEL 5
#define CAR_RIGHT_WHEEL 6

GLfloat car_body[4][2] = { { -16.0, -8.0 },{ -16.0, 0.0 },{ 16.0, 0.0 },{ 16.0, -8.0 } };
GLfloat car_frame[4][2] = { { -10.0, 0.0 },{ -10.0, 10.0 },{ 10.0, 10.0 },{ 10.0, 0.0 } };
GLfloat car_window[4][2] = { { -8.0, 0.0 },{ -8.0, 8.0 },{ 8.0, 8.0 },{ 8.0, 0.0 } };
GLfloat car_left_light[4][2] = { { -9.0, -6.0 },{ -10.0, -5.0 },{ -9.0, -4.0 },{ -8.0, -5.0 } };
GLfloat car_right_light[4][2] = { { 9.0, -6.0 },{ 8.0, -5.0 },{ 9.0, -4.0 },{ 10.0, -5.0 } };
GLfloat car_left_wheel[4][2] = { { -10.0, -12.0 },{ -10.0, -8.0 },{ -6.0, -8.0 },{ -6.0, -12.0 } };
GLfloat car_right_wheel[4][2] = { { 6.0, -12.0 },{ 6.0, -8.0 },{ 10.0, -8.0 },{ 10.0, -12.0 } };

GLfloat car_color[7][3] = {
	{ 0 / 255.0f, 149 / 255.0f, 159 / 255.0f },
	{ 0 / 255.0f, 149 / 255.0f, 159 / 255.0f },
	{ 216 / 255.0f, 208 / 255.0f, 174 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
	{ 21 / 255.0f, 30 / 255.0f, 26 / 255.0f },
	{ 21 / 255.0f, 30 / 255.0f, 26 / 255.0f }
};

GLuint VBO_car, VAO_car;
void prepare_car() {
	GLsizeiptr buffer_size = sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light) + sizeof(car_left_wheel) + sizeof(car_right_wheel);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car_body), car_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body), sizeof(car_frame), car_frame);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame), sizeof(car_window), car_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window), sizeof(car_left_light), car_left_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light),
		sizeof(car_right_light), car_right_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light), sizeof(car_left_wheel), car_left_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light)
		+ sizeof(car_right_light) + sizeof(car_left_wheel), sizeof(car_right_wheel), car_right_wheel);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car);
	glBindVertexArray(VAO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car() {
	glBindVertexArray(VAO_car);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_FRAME]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 4);

	glBindVertexArray(0);
}


//draw cocktail
#define COCKTAIL_NECK 0
#define COCKTAIL_LIQUID 1
#define COCKTAIL_REMAIN 2
#define COCKTAIL_STRAW 3
#define COCKTAIL_DECO 4

GLfloat neck[6][2] = { { -6.0, -12.0 },{ -6.0, -11.0 },{ -1.0, 0.0 },{ 1.0, 0.0 },{ 6.0, -11.0 },{ 6.0, -12.0 } };
GLfloat liquid[6][2] = { { -1.0, 0.0 },{ -9.0, 4.0 },{ -12.0, 7.0 },{ 12.0, 7.0 },{ 9.0, 4.0 },{ 1.0, 0.0 } };
GLfloat remain[4][2] = { { -12.0, 7.0 },{ -12.0, 10.0 },{ 12.0, 10.0 },{ 12.0, 7.0 } };
GLfloat straw[4][2] = { { 7.0, 7.0 },{ 12.0, 12.0 },{ 14.0, 12.0 },{ 9.0, 7.0 } };
GLfloat deco[8][2] = { { 12.0, 12.0 },{ 10.0, 14.0 },{ 10.0, 16.0 },{ 12.0, 18.0 },{ 14.0, 18.0 },{ 16.0, 16.0 },{ 16.0, 14.0 },{ 14.0, 12.0 } };

GLfloat cocktail_color[5][3] = {
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 0 / 255.0f, 63 / 255.0f, 122 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 191 / 255.0f, 255 / 255.0f, 0 / 255.0f },
	{ 218 / 255.0f, 165 / 255.0f, 32 / 255.0f }
};

GLuint VBO_cocktail, VAO_cocktail;
void prepare_cocktail() {
	GLsizeiptr buffer_size = sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw)
		+ sizeof(deco);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(neck), neck);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck), sizeof(liquid), liquid);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid), sizeof(remain), remain);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain), sizeof(straw), straw);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw),
		sizeof(deco), deco);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cocktail);
	glBindVertexArray(VAO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cocktail() {
	glBindVertexArray(VAO_cocktail);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_NECK]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_LIQUID]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_REMAIN]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_STRAW]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_DECO]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 8);

	glBindVertexArray(0);
}

//draw car2
#define CAR2_BODY 0
#define CAR2_FRONT_WINDOW 1
#define CAR2_BACK_WINDOW 2
#define CAR2_FRONT_WHEEL 3
#define CAR2_BACK_WHEEL 4
#define CAR2_LIGHT1 5
#define CAR2_LIGHT2 6

GLfloat car2_body[8][2] = { { -18.0, -7.0 },{ -18.0, 0.0 },{ -13.0, 0.0 },{ -10.0, 8.0 },{ 10.0, 8.0 },{ 13.0, 0.0 },{ 18.0, 0.0 },{ 18.0, -7.0 } };
GLfloat car2_front_window[4][2] = { { -10.0, 0.0 },{ -8.0, 6.0 },{ -2.0, 6.0 },{ -2.0, 0.0 } };
GLfloat car2_back_window[4][2] = { { 0.0, 0.0 },{ 0.0, 6.0 },{ 8.0, 6.0 },{ 10.0, 0.0 } };
GLfloat car2_front_wheel[8][2] = { { -11.0, -11.0 },{ -13.0, -8.0 },{ -13.0, -7.0 },{ -11.0, -4.0 },{ -7.0, -4.0 },{ -5.0, -7.0 },{ -5.0, -8.0 },{ -7.0, -11.0 } };
GLfloat car2_back_wheel[8][2] = { { 7.0, -11.0 },{ 5.0, -8.0 },{ 5.0, -7.0 },{ 7.0, -4.0 },{ 11.0, -4.0 },{ 13.0, -7.0 },{ 13.0, -8.0 },{ 11.0, -11.0 } };
GLfloat car2_light1[3][2] = { { -18.0, -1.0 },{ -17.0, -2.0 },{ -18.0, -3.0 } };
GLfloat car2_light2[3][2] = { { -18.0, -4.0 },{ -17.0, -5.0 },{ -18.0, -6.0 } };

GLfloat car2_color[7][3] = {
	{ 100 / 255.0f, 141 / 255.0f, 159 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f }
};


GLuint VBO_car2, VAO_car2;
void prepare_car2() {
	GLsizeiptr buffer_size = sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1) + sizeof(car2_light2);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car2_body), car2_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body), sizeof(car2_front_window), car2_front_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window), sizeof(car2_back_window), car2_back_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window), sizeof(car2_front_wheel), car2_front_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel),
		sizeof(car2_back_wheel), car2_back_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel), sizeof(car2_light1), car2_light1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1), sizeof(car2_light2), car2_light2);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car2);
	glBindVertexArray(VAO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car2() {
	glBindVertexArray(VAO_car2);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT1]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT2]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);

	glBindVertexArray(0);
}

// hat
#define HAT_LEAF 0
#define HAT_BODY 1
#define HAT_STRIP 2
#define HAT_BOTTOM 3

GLfloat hat_leaf[4][2] = { { 3.0, 20.0 },{ 3.0, 28.0 },{ 9.0, 32.0 },{ 9.0, 24.0 } };
GLfloat hat_body[4][2] = { { -19.5, 2.0 },{ 19.5, 2.0 },{ 15.0, 20.0 },{ -15.0, 20.0 } };
GLfloat hat_strip[4][2] = { { -20.0, 0.0 },{ 20.0, 0.0 },{ 19.5, 2.0 },{ -19.5, 2.0 } };
GLfloat hat_bottom[4][2] = { { 25.0, 0.0 },{ -25.0, 0.0 },{ -25.0, -4.0 },{ 25.0, -4.0 } };

GLfloat hat_color[4][3] = {
	{ 167 / 255.0f, 255 / 255.0f, 55 / 255.0f },
{ 255 / 255.0f, 144 / 255.0f, 32 / 255.0f },
{ 255 / 255.0f, 40 / 255.0f, 33 / 255.0f },
{ 255 / 255.0f, 144 / 255.0f, 32 / 255.0f }
};

GLuint VBO_hat, VAO_hat;

void prepare_hat() {
	GLsizeiptr buffer_size = sizeof(hat_leaf) + sizeof(hat_body) + sizeof(hat_strip) + sizeof(hat_bottom);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_hat);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hat);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hat_leaf), hat_leaf);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf), sizeof(hat_body), hat_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf) + sizeof(hat_body), sizeof(hat_strip), hat_strip);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hat_leaf) + sizeof(hat_body) + sizeof(hat_strip), sizeof(hat_bottom), hat_bottom);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_hat);
	glBindVertexArray(VAO_hat);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hat);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_hat() {
	glBindVertexArray(VAO_hat);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_LEAF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_STRIP]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, hat_color[HAT_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);
}

// cake
#define CAKE_FIRE 0
#define CAKE_CANDLE 1
#define CAKE_BODY 2
#define CAKE_BOTTOM 3
#define CAKE_DECORATE 4

GLfloat cake_fire[4][2] = { { -0.5, 14.0 },{ -0.5, 13.0 },{ 0.5, 13.0 },{ 0.5, 14.0 } };
GLfloat cake_candle[4][2] = { { -1.0, 8.0 } ,{ -1.0, 13.0 },{ 1.0, 13.0 },{ 1.0, 8.0 } };
GLfloat cake_body[4][2] = { { 8.0, 5.0 },{ -8.0, 5.0 } ,{ -8.0, 8.0 },{ 8.0, 8.0 } };
GLfloat cake_bottom[4][2] = { { -10.0, 1.0 },{ -10.0, 5.0 },{ 10.0, 5.0 },{ 10.0, 1.0 } };
GLfloat cake_decorate[4][2] = { { -10.0, 0.0 },{ -10.0, 1.0 },{ 10.0, 1.0 },{ 10.0, 0.0 } };

GLfloat cake_color[5][3] = {
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 204 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 102 / 255.0f, 51 / 255.0f, 0 / 255.0f }
};

GLuint VBO_cake, VAO_cake;

void prepare_cake() {
	int size = sizeof(cake_fire);
	GLsizeiptr buffer_size = sizeof(cake_fire) * 5;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, cake_fire);
	glBufferSubData(GL_ARRAY_BUFFER, size, size, cake_candle);
	glBufferSubData(GL_ARRAY_BUFFER, size * 2, size, cake_body);
	glBufferSubData(GL_ARRAY_BUFFER, size * 3, size, cake_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, size * 4, size, cake_decorate);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cake);
	glBindVertexArray(VAO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cake() {
	glBindVertexArray(VAO_cake);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_FIRE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_CANDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_DECORATE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glBindVertexArray(0);
}

// sword

#define SWORD_BODY 0
#define SWORD_BODY2 1
#define SWORD_HEAD 2
#define SWORD_HEAD2 3
#define SWORD_IN 4
#define SWORD_DOWN 5
#define SWORD_BODY_IN 6

GLfloat sword_body[4][2] = { { -6.0, 0.0 },{ -6.0, -4.0 },{ 6.0, -4.0 },{ 6.0, 0.0 } };
GLfloat sword_body2[4][2] = { { -2.0, -4.0 },{ -2.0, -6.0 } ,{ 2.0, -6.0 },{ 2.0, -4.0 } };
GLfloat sword_head[4][2] = { { -2.0, 0.0 },{ -2.0, 16.0 } ,{ 2.0, 16.0 },{ 2.0, 0.0 } };
GLfloat sword_head2[3][2] = { { -2.0, 16.0 },{ 0.0, 19.46 } ,{ 2.0, 16.0 } };
GLfloat sword_in[4][2] = { { -0.3, 0.7 },{ -0.3, 15.3 } ,{ 0.3, 15.3 },{ 0.3, 0.7 } };
GLfloat sword_down[4][2] = { { -2.0, -6.0 } ,{ 2.0, -6.0 },{ 4.0, -8.0 },{ -4.0, -8.0 } };
GLfloat sword_body_in[4][2] = { { 0.0, -1.0 } ,{ 1.0, -2.732 },{ 0.0, -4.464 },{ -1.0, -2.732 } };

GLfloat sword_color[7][3] = {
	{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_sword, VAO_sword;

void prepare_sword() {
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword() {
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

	glBindVertexArray(0);
}

/* ----- Defined functions for collision detection----- */


bool aabb_collision(const AABB& a, const AABB& b) {
	// Check for overlap in X and Y axes
	bool overlapX = a.maxX >= b.minX && a.minX <= b.maxX;
	bool overlapY = a.maxY >= b.minY && a.minY <= b.maxY;

	return overlapX && overlapY;
}

AABB calculate_world_aabb(const AABB& local_aabb, const glm::vec2& pos, float scale) {
	AABB world_aabb;
	world_aabb.minX = pos.x + local_aabb.minX * scale;
	world_aabb.minY = pos.y + local_aabb.minY * scale;
	world_aabb.maxX = pos.x + local_aabb.maxX * scale;
	world_aabb.maxY = pos.y + local_aabb.maxY * scale;
	return world_aabb;
}

AABB calculate_rotated_world_aabb(const AABB& local_aabb, const glm::vec2& world_pos, float scale, float angle_rad) {
	// Calculate corners in local space, scaled
	glm::vec2 corners[4] = {
		{local_aabb.minX * scale, local_aabb.minY * scale},
		{local_aabb.maxX * scale, local_aabb.minY * scale},
		{local_aabb.maxX * scale, local_aabb.maxY * scale},
		{local_aabb.minX * scale, local_aabb.maxY * scale}
	};

	// Rotate corners and find min/max in world space
	float cos_a = std::cos(angle_rad);
	float sin_a = std::sin(angle_rad);
	AABB world_aabb;
	world_aabb.minX = world_aabb.minY = std::numeric_limits<float>::max();
	world_aabb.maxX = world_aabb.maxY = -std::numeric_limits<float>::max();

	for (int i = 0; i < 4; ++i) {
		float rotated_x = corners[i].x * cos_a - corners[i].y * sin_a;
		float rotated_y = corners[i].x * sin_a + corners[i].y * cos_a;

		float world_x = world_pos.x + rotated_x;
		float world_y = world_pos.y + rotated_y;

		world_aabb.minX = std::min(world_aabb.minX, world_x);
		world_aabb.maxX = std::max(world_aabb.maxX, world_x);
		world_aabb.minY = std::min(world_aabb.minY, world_y);
		world_aabb.maxY = std::max(world_aabb.maxY, world_y);
	}
	return world_aabb;
}

// helper function to add car objects
void add_car(glm::vec2 position, float car_scale, float car2_scale) {
	// Create Base Car
	CarInstance car;
	car.position = position;
	car.scale = car_scale;
	car.active = true;
	// Calculate initial world AABB for the car
	car.world_aabb = calculate_world_aabb(CAR_AABB, car.position, car.scale);
	car.child_car2s.clear();

	// Create Dependent Car2s for this Car
	for (int j = 0; j < 3; j++) {
		Car2Instance car2;
		car2.scale = car2_scale; // Car2 scale is relative to parent
		car2.rotation_angle = (j * 120.0f) * TO_RADIAN; // Initial relative angle
		car2.parent_rotation_offset = 0.0f;
		car2.active = true;

		// Calculate initial relative position based on parent and rotation
		float total_angle_rad = car2.rotation_angle + car2.parent_rotation_offset;
		car2.relative_position.x = car2_relative_dist * std::cos(total_angle_rad);
		car2.relative_position.y = car2_relative_dist * std::sin(total_angle_rad);

		// Calculate initial world position and AABB for car2
		glm::vec2 world_pos = car.position + car2.relative_position; // World pos = parent pos + relative pos
		float world_scale = car2.scale * car.scale; // World scale = relative scale * parent scale
		car2.world_aabb = calculate_rotated_world_aabb(CAR2_AABB, world_pos, world_scale, total_angle_rad);

		car.child_car2s.push_back(car2); // Add child to parent's vector
	}
	active_cars.push_back(car);
	object_count++;
}

void add_cocktail(glm::vec2 position, float scale) {
	CocktailInstance cocktail;
	cocktail.position = position;
	cocktail.current_scale = scale;
	cocktail.active = true;
	cocktail.world_aabb = calculate_world_aabb(COCKTAIL_AABB, cocktail.position, cocktail.current_scale);
	active_cocktails.push_back(cocktail);
	object_count++;
}

void initialize_objects() {
	// Clear existing objects
	active_cars.clear();
	active_cocktails.clear();
	active_hadoukens.clear();
	object_count = 0;

	// Initialize Cars and their dependent Car2s
	for (int i = 0; i < 3; i++) {
		add_car(glm::vec2(600.0f, i * 180.0f - 320.0f), car_scale, car2_scale);
	}

	// Store initial positions for other resettable objects
	initial_airplane_x = win_width > 0 ? win_width / 2.0f : 300.0f;
	initial_airplane_y = 0.0f; // Assuming starts at y=0 of its sine wave path
	initial_airplane_angle = 0.0f; // Or calculate initial angle if needed
	initial_ryu_pos_x = -400.0f;
	initial_ryu_pos_y = -200.0f;
	ryu_state = RYU_IDLE;
	ryu_current_health = ryu_max_health;
}

void reset_simulation() {
	printf("Resetting...\n");

	// Re-initialize objects
	initialize_objects(); 

	// Reset other dynamic elements
	airplane_x = initial_airplane_x;
	airplane_y = initial_airplane_y;
	airplane_angle = initial_airplane_angle;
	ryu_pos_x = initial_ryu_pos_x;
	ryu_pos_y = initial_ryu_pos_y;
	ryu_current_health = ryu_max_health;
	is_dragging_ryu = false;
	car_speed = -0.5f;	//easy mode

	// Reset timers/cooldowns
	last_hadouken_time = 0;
	start_time = glutGet(GLUT_ELAPSED_TIME); // Reset animation timer base

	glutPostRedisplay();
}

void display(void) {
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_skybox();

	/*
	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();
	*/

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-560.0f, 399.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.2f, 6.2f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_healthbar_border();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3( -550.0f, 400.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(ryu_max_health / 100 * 5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_healthbar();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-550.0f, 400.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3((ryu_max_health - ryu_current_health) / 100 * 5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_healthbar_red();

	// draw airplane's position according to animation)
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(airplane_x, airplane_y + 200.0f, 0.0f));		// added more height to y coordinates
	ModelMatrix = glm::rotate(ModelMatrix, airplane_angle + 90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	/*
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_shirt();
	*/

	for (int i = 0; i < 16; i++) {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 150.0f - 1000.0f + airplane_x, skybox_y + 20.0f, 0.0f));
		int scale = rand() % 5;
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.8f + 0.2f * (float)scale , 2.8f + 0.2f * (float)scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_house();
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(ryu_pos_x, ryu_pos_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(ryu_scale, ryu_scale, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	if (ryu_state == RYU_IDLE)
		draw_ryu();
	else draw_ryu_fire();
	// Draw ryu AABB (debug)
	if (game_mode == DEBUG_MODE) draw_aabb_box(ryu_aabb);

	for (const auto& hadouken : active_hadoukens) {
		if (!hadouken.active) continue;
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hadouken.position.x, hadouken.position.y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(hadouken.current_scale, HADOUKEN_SCALE, 1.0f)); // Apply scale on x-axis only
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_hadouken();
		// Draw Hadouken AABB (debug)
		if (game_mode == DEBUG_MODE) draw_aabb_box(hadouken.world_aabb);
	}

	// hierarchical modeling of car2 around car
	// Draw active Cars
	for (const auto& car : active_cars) {
		if (!car.active) continue;
		glm::mat4 ParentModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car.position.x, car.position.y, 0.0f));
		ParentModelMatrix = glm::scale(ParentModelMatrix, glm::vec3(car.scale, car.scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ParentModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_car();
		// Draw Parent Car AABB (debug)
		if (game_mode == DEBUG_MODE) draw_aabb_box(car.world_aabb);

		for (const auto& car2 : car.child_car2s) {
			if (!car2.active) continue; // Skip inactive children

			// Calculate child's world transformation relative to the parent's current transformation
			float total_angle_rad = car2.rotation_angle + car2.parent_rotation_offset;
			glm::vec2 world_pos = car.position + car2.relative_position; // Use pre-calculated relative pos
			float world_scale = car2.scale * car.scale;

			// Start with the parent's world matrix
			// Apply child's rotation (around parent's origin), translation (relative), and scale (relative)
			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(world_pos.x, world_pos.y, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, total_angle_rad, glm::vec3(0.0f, 0.0f, 1.0f));
			// Apply the final calculated world scale
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(world_scale, world_scale, 1.0f));

			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_car2(); // Draw the child car2 geometry
			// Draw Child Car2 AABB (debug)
			if (game_mode == DEBUG_MODE) draw_aabb_box(car2.world_aabb);
		}
	}

	// draw cocktail 
	for (const auto& cocktail : active_cocktails) {
		if (!cocktail.active) continue;
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cocktail.position.x, cocktail.position.y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail.current_scale, cocktail.current_scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cocktail();
		if (game_mode == DEBUG_MODE) draw_aabb_box(cocktail.world_aabb);
	}

	/*
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -100.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hat();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-400.0f, -100.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cake();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, -100.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();
	*/
	glFlush();
}

void timer_func(int value) {
	int current_time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed_time = current_time - start_time;

	// --- Game State Logic ---
	if (ryu_current_health <= 0.0f)
		reset_simulation();


	// add cars or cocktails (90/10 chance) on random locations if there is space
	if (object_count < 3) {
		if (rand() % 10 < 9) {
			add_car(glm::vec2(600.0f, (rand() % 3) * 180.0f - 320.0f), car_scale, car2_scale);
		}
		else add_cocktail(glm::vec2(600.0f, (rand() % 3) * 180.0f - 320.0f), cocktail_base_scale);
	}

	// --- Airplane Update ---
	// Calculate animation progress (0.0 to 1.0)
	float progress = (float)elapsed_time / (float)airplane_animation_duration;
	if (progress > 1.0f) {
		progress = 1.0f; // Clamp progress to 1.0
		start_time = glutGet(GLUT_ELAPSED_TIME); // Loop the animation
	}

	// Calculate airplane's X position (right to left)
	// Start at right edge (+win_width/2), end at left edge (-win_width/2)
	float start_x = win_width / 2.0f;
	float end_x = -win_width / 2.0f;
	airplane_x = start_x + (end_x - start_x) * progress;

	// Calculate airplane's Y position using sine function
	// The input to sin() goes from 0 to frequency * 2 * PI over the duration
	float angle_rad = sine_frequency * progress * 2.0f * (float)PI;
	airplane_y = sine_amplitude * sin(angle_rad);

	// Calculate airplane's angle using atan function
	float dy_danglerad = sine_amplitude * cos(angle_rad);
	float danglerad_dprogress = sine_frequency * 2.0f * (float)PI;
	float dy_dprogress = dy_danglerad * danglerad_dprogress;

	float dx_dprogress = (end_x - start_x);
	// Ensure dx_dprogress is not zero if win_width hasn't been set yet
	if (win_width == 0) dx_dprogress = -1.0f;
	airplane_angle = atan2(dy_dprogress, dx_dprogress);

	// --- Ryu Update ---
	if (current_time - last_hadouken_time <= HADOUKEN_COOLDOWN_MS)
		ryu_state = RYU_FIRE;
	else ryu_state = RYU_IDLE;
	ryu_aabb = calculate_world_aabb(RYU_AABB, { ryu_pos_x, ryu_pos_y }, ryu_scale);

	// Screen edge (for car and hadouken spawn / despawn)
	float screen_right_edge = win_width > 0 ? win_width / 2.0f : 600.0f; // Right edge boundary
	float screen_left_edge = -screen_right_edge;

	// --- Cocktail Update ---
	float pulsate_progress = fmod((float)current_time, cocktail_pulsate_period_ms) / cocktail_pulsate_period_ms;
	float current_pulsation = sin(pulsate_progress * 2.0f * PI);
	for (auto& cocktail : active_cocktails) {
		if (!cocktail.active) continue;

		cocktail.position.x += cocktail_speed;
		cocktail.current_scale = cocktail_base_scale + cocktail_pulsate_amplitude * current_pulsation;
		cocktail.world_aabb = calculate_world_aabb(COCKTAIL_AABB, cocktail.position, cocktail.current_scale);
		if (cocktail.position.x < screen_left_edge - 10.0f) {	// if cocktail hits left side of screen
			cocktail.active = false;
		}
	}

	// --- Car Update ---
	for (auto& car : active_cars) {
		if (!car.active) continue;

		// Update Parent Car's AABB (redundant for now, but for when parent moves)
		car.position.x += car_speed;
		if (car.position.x < screen_left_edge - 10.0f) {	// if car hits left side of screen
			car.active = false;
			ryu_current_health -= 50.0f;					// -50 health as punish
		}
		car.world_aabb = calculate_world_aabb(CAR_AABB, car.position, car.scale);

		// Update Children
		for (auto& car2 : car.child_car2s) {
			if (!car2.active) continue;

			// Update dynamic rotation offset
			car2.rotation_angle += car2RotationOffset * TO_RADIAN;
			if (car2.rotation_angle >= 360.0f) car2.rotation_angle -= 360.0f;

			// Recalculate relative position based on total angle
			float total_angle_rad = car2.rotation_angle + car2.parent_rotation_offset;
			car2.relative_position.x = car2_relative_dist * std::cos(total_angle_rad);
			car2.relative_position.y = car2_relative_dist * std::sin(total_angle_rad);

			// Recalculate world position and AABB
			glm::vec2 world_pos = car.position + car2.relative_position; // World = Parent + Relative
			float world_scale = car2.scale * car.scale; // Child World Scale = Child Relative Scale * Parent World Scale
			car2.world_aabb = calculate_rotated_world_aabb(CAR2_AABB, world_pos, world_scale, total_angle_rad);
		}
	}

	// --- Hadouken Update ---
	// Update position for each active Hadouken
	for (auto& hadouken : active_hadoukens) {
		if (!hadouken.active) continue;
		hadouken.position.x += HADOUKEN_SPEED;
		hadouken.current_scale = HADOUKEN_SCALE + cocktail_pulsate_amplitude * current_pulsation;
		// Check if off-screen
		if (hadouken.position.x > screen_right_edge + 50.0f) { // Add buffer
			hadouken.active = false;
		}
		// Update Hadouken world AABB
		hadouken.world_aabb = calculate_world_aabb(HADOUKEN_AABB, hadouken.position, HADOUKEN_SCALE);
	}


	// --- Collision Detection ---

	// Ryu health logic
	for (auto& car : active_cars) {
		if (!car.active) continue; // Skip inactive cars entirely

		// 1. Check collision with Parent Car
		if (aabb_collision(ryu_aabb, car.world_aabb)) {
			car.active = false; // Deactivate parent car
			// Deactivate all children of this car
			for (auto& child_car2 : car.child_car2s) {
				child_car2.active = false;
			}
			ryu_current_health -= 40.0f;	// 40 damage
		}

		// 2. Check collision with Children (only if parent wasn't hit)
		for (auto& car2 : car.child_car2s) {
			if (!car2.active) continue; // Skip inactive children

			if (aabb_collision(ryu_aabb, car2.world_aabb)) {
				car2.active = false; // Deactivate only the child
				ryu_current_health -= 10.0f;	// 10 damage
			}
		}
	}
	// 3. Check collision with Cocktail
	for (auto& cocktail : active_cocktails) {
		if (!cocktail.active) continue;

		if (aabb_collision(ryu_aabb, cocktail.world_aabb)) {
			cocktail.active = false;
			ryu_current_health += 30.0f;		// 30 heal
			if (ryu_current_health > ryu_max_health) 
				ryu_current_health = ryu_max_health;
		}
	}

	// hadouken logic
	for (auto& hadouken : active_hadoukens) {
		if (!hadouken.active) continue;

		for (auto& car : active_cars) {
			if (!car.active) continue; // Skip inactive cars entirely

			// 1. Check collision with Parent Car
			if (aabb_collision(hadouken.world_aabb, car.world_aabb)) {
				hadouken.active = false;
				car.active = false; // Deactivate parent car
				// Deactivate all children of this car
				for (auto& child_car2 : car.child_car2s) {
					child_car2.active = false;
				}
				goto next_hadouken; // Hadouken is gone, check next hadouken
			}

			// 2. Check collision with Children (only if parent wasn't hit)
			for (auto& car2 : car.child_car2s) {
				if (!car2.active) continue; // Skip inactive children

				if (aabb_collision(hadouken.world_aabb, car2.world_aabb)) {
					hadouken.active = false;
					car2.active = false; // Deactivate only the child
					goto next_hadouken; // Hadouken is gone, check next hadouken
				}
			}
		}
	next_hadouken:; // Label for jumping to next hadouken check
	}

	// --- Remove Inactive Objects ---
	active_hadoukens.erase(
		std::remove_if(active_hadoukens.begin(), active_hadoukens.end(),
			[](const Hadouken& h) { return !h.active; }),
		active_hadoukens.end());

	// Remove inactive cocktails
	active_cocktails.erase(
		std::remove_if(active_cocktails.begin(), active_cocktails.end(),
			[](const CocktailInstance& c) { return !c.active; }),
		active_cocktails.end());

	// Remove inactive Cars (parents) - their children vectors are removed with them
	active_cars.erase(
		std::remove_if(active_cars.begin(), active_cars.end(),
			[](const CarInstance& c) { return !c.active; }),
		active_cars.end());

	// Remove inactive Car2s (children) from remaining active Cars
	for (auto& car : active_cars) {
		// No need to check car.active here, as inactive cars were already removed
		car.child_car2s.erase(
			std::remove_if(car.child_car2s.begin(), car.child_car2s.end(),
				[](const Car2Instance& c2) { return !c2.active; }),
			car.child_car2s.end());
	}

	// Update total object count
	object_count = active_cars.size() + active_cocktails.size();

	glutPostRedisplay();

	glutTimerFunc(16, timer_func, 0);	// timer_func called every 16ms (60fps)
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			float world_x = (float)x - win_width / 2.0f;
			float world_y = win_height / 2.0f - (float)y;
			ryu_aabb = calculate_world_aabb(RYU_AABB, { ryu_pos_x, ryu_pos_y }, ryu_scale);
			if (world_x >= ryu_aabb.minX && world_x <= ryu_aabb.maxX &&
				world_y >= ryu_aabb.minY && world_y <= ryu_aabb.maxY) {
				is_dragging_ryu = true;
				prev_mouse_x = x;
				prev_mouse_y = y;
			}
		}
		else if (state == GLUT_UP) {
			is_dragging_ryu = false;
		}
	}
}

void motion(int x, int y) {
	if (is_dragging_ryu) {
		int delta_x = x - prev_mouse_x;
		int delta_y = y - prev_mouse_y;
		float delta_world_x = (float)delta_x;
		float delta_world_y = -(float)delta_y;
		ryu_pos_x += delta_world_x;
		ryu_pos_y += delta_world_y;
		if (ryu_pos_y > skybox_y) ryu_pos_y = skybox_y;		// Clamp ryu's y coord to ground
		prev_mouse_x = x;
		prev_mouse_y = y;
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case '1':
		car_speed = -0.5f;		// easy mode
		break;
	case '2':
		car_speed = -1.5f;		// normal mode
		break;
	case '3':
		car_speed = -3.0f;		// hard mode
		break;
	case 'h':
	case 'H':
	{
		//ryu_current_health = 30;
		int current_time = glutGet(GLUT_ELAPSED_TIME);
		// Check if cooldown has passed
		if (current_time - last_hadouken_time >= HADOUKEN_COOLDOWN_MS) {
			// Create and add a new Hadouken
			ryu_state = RYU_FIRE;
			Hadouken new_hadouken;
			new_hadouken.position = glm::vec2(ryu_pos_x + HADOUKEN_START_OFFSET_X, ryu_pos_y + HADOUKEN_START_OFFSET_Y);
			new_hadouken.current_scale = HADOUKEN_SCALE;
			new_hadouken.world_aabb = calculate_world_aabb(HADOUKEN_AABB, new_hadouken.position, HADOUKEN_SCALE);
			new_hadouken.active = true;
			active_hadoukens.push_back(new_hadouken);

			// Update the last fired time
			last_hadouken_time = current_time;
			printf("Hadouken!\n");
		}
		break;
	}
	case 'd':
	case 'D':
		game_mode = 1 - game_mode;
		break;
	case 'r':
	case 'R':
		reset_simulation();
	break;
	}
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();
	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	// Delete others here too!!!
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(98 / 255.0f, 181 / 255.0f, 120 / 255.0f, 1.0f);	// changed bg color to grass-ish color
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_line();
	prepare_skybox();
	prepare_aabb_box();
	prepare_healthbar();
	prepare_ryu();
	prepare_ryu_fire();
	prepare_hadouken();
	prepare_airplane();
	//prepare_shirt();
	prepare_house();
	prepare_car();
	prepare_cocktail();
	prepare_car2();
	//prepare_hat();
	//prepare_cake();
	//prepare_sword();
	initialize_objects();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 5
int main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 2DObjects_GLSL_3.0.1.3";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' (Quit), 'r' (Reset Simulation), 'h' (Fire Hadouken), 'd' (Debug mode, see hitboxes), '1 2 3' (Easy, Normal, Hard mode)",
		"    - Mouse: Left-click and drag Ryu around",
		"    - Colliding with cars or letting them reach the end will damage you!",
		"	 - Hit Cars with hadouken to stop them from reaching the end!",
		"    - Move towards cocktails to heal Ryu's health!"
	};

	srand((unsigned int)time(NULL));
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(win_width, win_height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	start_time = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(0, timer_func, 0);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();

	return 0;
}

