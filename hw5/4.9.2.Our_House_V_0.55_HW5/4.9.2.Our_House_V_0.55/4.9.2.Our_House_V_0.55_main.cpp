#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shaders/LoadShaders.h"
#include "Scene_Definitions.h"
#include <iostream>
Scene scene;

// Global variables for mouse input and delta time calculation
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static bool firstMouse = true;
static float lastX_mouse = 1200.0f / 2.0f; // Initial window width / 2
static float lastY_mouse = 800.0f / 2.0f;  // Initial window height / 2
static bool mouseLookActive = false;     // Toggled by 'm' key
const float ARROW_KEY_LOOK_SENSITIVITY = 2.5f;	// For controlling cam4

// callback functions
void mouse_button_callback(int button, int state, int x, int y);
void mouse_motion_callback(int x, int y); // For when mouse look is active
void mouse_wheel_callback(int wheel, int direction, int x, int y);

// for lights
void initialize_lights();

Perspective_Camera* getControlledCamera() {
	// Attempt to get the main perspective camera that is marked as movable
	for (auto& cam_iter : scene.camera_list) {
		Camera& cam = cam_iter.get();
		if (cam.camera_id == CAMERA_MAIN && cam.flag_move) {
			return dynamic_cast<Perspective_Camera*>(&cam);
		}
	}
	// Fallback: directly try to get from scene.camera_data if specific ID needed and not found above
	// This assumes scene.camera_data.cam_main is the one intended for control
	if (scene.camera_data.cam_main.flag_move) {
		return &scene.camera_data.cam_main;
	}
	return nullptr;
}

void display(void) {
	// Calculate delta time for frame-rate independent movement
	float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Time in seconds
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	Camera* main_cam_ptr = nullptr;
	for (auto& cam_ref : scene.camera_list) {
		if (cam_ref.get().camera_id == CAMERA_MAIN) {
			main_cam_ptr = &cam_ref.get();
			break;
		}
	}

	if (main_cam_ptr) {
		// The ViewMatrix transforms from World to Camera space. Its inverse transforms from Camera to World.
		glm::mat4 camera_world_transform = glm::inverse(main_cam_ptr->ViewMatrix);

		// The camera's position in the world is the 4th column of the inverse view matrix.
		glm::vec3 cam_pos_world = glm::vec3(camera_world_transform[3]);

		// The camera's forward direction in the world is the negative Z-axis of its transform.
		glm::vec3 cam_dir_world = -glm::vec3(camera_world_transform[2]);

		// Update the light properties in the scene. These are in World Coordinates.
		// They will be converted to Eye Coordinates inside the draw_object functions.
		scene.lights[2].position = glm::vec4(cam_pos_world, 1.0f);
		scene.lights[2].spot_direction = glm::normalize(cam_dir_world);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto camera = scene.camera_list.begin(); camera != scene.camera_list.end(); camera++) {
		Camera& current_cam = camera->get();	// refactored to stop referencing iter every call
		if (current_cam.flag_valid == false) continue;
		glViewport(current_cam.view_port.x, current_cam.view_port.y,
			current_cam.view_port.w, current_cam.view_port.h);
		scene.ViewMatrix = current_cam.ViewMatrix;
		scene.ProjectionMatrix = current_cam.ProjectionMatrix;

		scene.draw_world();

		//drawing individual camera axis if flag is set to true
		if (scene.draw_axis_flag) {
			Shader_Simple* shader_simple_ptr = static_cast<Shader_Simple*>(&scene.shader_list[shader_ID_mapper[SHADER_SIMPLE]].get());
			#define CAMERA_AXIS_VIS_LENGTH 15.0f // Define a suitable length for visualizing camera axes
			for (auto& cam_to_visualize_ref : scene.camera_list) {
				Camera& cam_to_visualize = cam_to_visualize_ref.get();
				glm::mat4 camera_world_transform = glm::inverse(cam_to_visualize.ViewMatrix);
				glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(CAMERA_AXIS_VIS_LENGTH, CAMERA_AXIS_VIS_LENGTH, CAMERA_AXIS_VIS_LENGTH));
				glm::mat4 axis_model_matrix = camera_world_transform * scale_matrix;
				scene.axis_object.draw_axis(
					shader_simple_ptr,
					current_cam.ViewMatrix,    // Current view matrix (from the outer loop)
					current_cam.ProjectionMatrix, // Current projection matrix (from the outer loop)
					axis_model_matrix
				);
			}
		}
	}
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0, polygon_fill_on = 0, depth_test_on = 0;
	Perspective_Camera* controlledCam = getControlledCamera();

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;
		}
		break;
	case 'f':
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;
	case 't':
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;
	case 'm':
		mouseLookActive = !mouseLookActive;	// toggle mouse look
		if (mouseLookActive) {
			glutSetCursor(GLUT_CURSOR_NONE); // Hide cursor
			fprintf(stdout, "^^^ Mouse look enabled. Move mouse to look. Press 'M' again to disable.\n");
			firstMouse = true; // Reset mouse position on mode activation
			// Center mouse cursor
			glutWarpPointer(scene.window.width / 2, scene.window.height / 2);
		}
		else {
			glutSetCursor(GLUT_CURSOR_INHERIT); // Show cursor
			fprintf(stdout, "^^^ Mouse look disabled.\n");
		}
		break;
	case 'x':	// toggle axis draw
		scene.draw_axis_flag = !scene.draw_axis_flag;
		if (scene.draw_axis_flag) {
			fprintf(stdout, "^^^ Enabling camera local axes.\n");
		}
		else {
			fprintf(stdout, "^^^ Disabling camera local axes.\n");
		}
		glutPostRedisplay();
		break;
		// Camera movement keys
	case 'w': case 'W':
	case 's': case 'S':
	case 'a': case 'A':
	case 'd': case 'D':
	case 'q': case 'Q':
	case 'e': case 'E':
		if (controlledCam) {
			controlledCam->ProcessKeyboardMovement(tolower(key), deltaTime);
			glutPostRedisplay();
		}
		break;
	case 'u':
		scene.set_active_cc_camera(CAMERA_CC_0);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Static Projection Camera 1.\n");
		break;
	case 'i':
		scene.set_active_cc_camera(CAMERA_CC_1);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Static Projection Camera 2.\n");
		break;
	case 'o':
		scene.set_active_cc_camera(CAMERA_CC_2);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Static Projection Camera 3.\n");
		break;
	case 'p':
		scene.set_active_cc_camera(CAMERA_CC_MOVE);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Moving Projection Camera.\n");
		break;
	case 'h':
		scene.set_active_cc_camera(CAMERA_SIDE_FRONT);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Front-View Camera.\n");
		break;
	case 'j':
		scene.set_active_cc_camera(CAMERA_TOP);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Top-View Camera.\n");
		break;
	case 'k':
		scene.set_active_cc_camera(CAMERA_SIDE);
		glutPostRedisplay();
		fprintf(stdout, "^^^ Switched to Side-View Camera.\n");
		break;
	case '1':
		scene.shader_kind = SHADER_GOURAUD;
		fprintf(stdout, "^^^ Gouraud Shading ON.\n");
		glutPostRedisplay();
		break;
	case '2':
		scene.shader_kind = SHADER_PHONG;
		fprintf(stdout, "^^^ Phong Shading ON.\n");
		glutPostRedisplay();
		break;
	case '3':
		scene.lights[0].light_on = !scene.lights[0].light_on;
		if (scene.lights[0].light_on)
			fprintf(stdout, "^^^ Turned Main Light ON.\n");
		else fprintf(stdout, "^^^ Turned Main Light OFF.\n");
		glutPostRedisplay();
		break;
	case '4':
		scene.lights[2].light_on = !scene.lights[2].light_on;
		if (scene.lights[2].light_on)
			fprintf(stdout, "^^^ Turned Flashlight ON.\n");
		else fprintf(stdout, "^^^ Turned Flashlight OFF.\n");
		glutPostRedisplay();
		break;
	case '5':
		scene.lights[1].light_on = !scene.lights[1].light_on;
		if (scene.lights[1].light_on)
			fprintf(stdout, "^^^ Turned Wolf Light ON.\n");
		else fprintf(stdout, "^^^ Turned Wolf Light OFF.\n");
		glutPostRedisplay();
		break;
	}

}

// keyboard callback for cam4 movement
void keyboard_special(int key, int x, int y) {
	// Check if the active secondary camera is CAMERA_CC_MOVE
	if (scene.current_active_cc_cam_id == CAMERA_CC_MOVE) {

		Camera* temp_cam_ptr = nullptr;
		for (auto& cam_ref : scene.camera_list) {
			if (cam_ref.get().camera_id == CAMERA_CC_MOVE) {
				temp_cam_ptr = &cam_ref.get();
				break;
			}
		}

		Perspective_Camera* cc_move_cam = dynamic_cast<Perspective_Camera*>(temp_cam_ptr);

		if (cc_move_cam) {
			bool view_changed = false;
			switch (key) {
			case GLUT_KEY_UP:
				cc_move_cam->Pitch += ARROW_KEY_LOOK_SENSITIVITY;
				view_changed = true;
				break;
			case GLUT_KEY_DOWN:
				cc_move_cam->Pitch -= ARROW_KEY_LOOK_SENSITIVITY;
				view_changed = true;
				break;
			case GLUT_KEY_LEFT:
				cc_move_cam->Yaw += ARROW_KEY_LOOK_SENSITIVITY; // Invert movement
				view_changed = true;
				break;
			case GLUT_KEY_RIGHT:
				cc_move_cam->Yaw -= ARROW_KEY_LOOK_SENSITIVITY;
				view_changed = true;
				break;
			}

			if (view_changed) {
				if (cc_move_cam->Pitch > 89.0f) cc_move_cam->Pitch = 89.0f;
				if (cc_move_cam->Pitch < -89.0f) cc_move_cam->Pitch = -89.0f;

				cc_move_cam->updateCameraVectors();
				cc_move_cam->RecomputeViewMatrix();
				glutPostRedisplay();
			}
		}
	}
	// You might want to pass other special keys to the main camera if it uses them
}

void reshape(int width, int height) {
	scene.window.width = width;
	scene.window.height = height;
	scene.window.aspect_ratio = (float)width / height;
	//scene.create_camera_list(scene.window.width, scene.window.height, scene.window.aspect_ratio);

	// Update aspect ratio for all perspective cameras in the scene's list
	for (auto& cam_ref_wrapper : scene.camera_list) {
		Camera& cam = cam_ref_wrapper.get();
		if (cam.flag_valid && cam.cam_proj.projection_type == CAMERA_PROJECTION_PERSPECTIVE) {
			cam.RecomputeProjectionMatrix(scene.window.aspect_ratio);
		}
	}

	glutPostRedisplay();
}

void timer_scene(int index) {
	scene.clock(0);
	glutPostRedisplay();
	glutTimerFunc(16, timer_scene, 0); // 60fps baby!!
}

// GLUT Mouse Button Callbacks

void mouse_button_callback(int button, int state, int x, int y) {
	// Example: if you want to tie mouseLookActive to a button press (e.g. right mouse button)
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
	     mouseLookActive = true;
	     glutSetCursor(GLUT_CURSOR_NONE);
	     firstMouse = true;
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
	     mouseLookActive = false;
	     glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

// GLUT Mouse Motion Callback (called when mouse moves WHILE a button is pressed)
// For continuous look without button press, use glutPassiveMotionFunc
void mouse_motion_callback(int x, int y) {
	if (!mouseLookActive) return;

	Perspective_Camera* controlledCam = getControlledCamera();
	if (!controlledCam) return;

	if (firstMouse) {
		lastX_mouse = (float)x;
		lastY_mouse = (float)y;
		firstMouse = false;
	}

	float xoffset = (float)x - lastX_mouse;
	float yoffset = lastY_mouse - (float)y; // Reversed: y-coordinates go from top to bottom

	lastX_mouse = (float)x;
	lastY_mouse = (float)y;

	controlledCam->ProcessMouseMovement(xoffset, yoffset);
	glutPostRedisplay();
}

// GLUT Mouse Wheel Callback
void mouse_wheel_callback(int wheel, int direction, int x, int y) {
	// wheel: identifier of the wheel (usually 0)
	// direction: 1 for scroll up (or forward), -1 for scroll down (or backward)
	// x, y: mouse coordinates at the time of the event

	Perspective_Camera* controlledCam = getControlledCamera();
	if (controlledCam) {
		controlledCam->ProcessMouseScroll((float)direction); // Pass direction as y-offset
		glutPostRedisplay();
	}
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);	// for arrow key handling
	glutReshapeFunc(reshape);
 	glutTimerFunc(16, timer_scene, 0);	// 60fps baby!!

	glutMouseFunc(mouse_button_callback);       // For mouse button events
	//glutMotionFunc(mouse_motion_callback);      // For mouse movement when a button is pressed
	glutPassiveMotionFunc(mouse_motion_callback); // For mouse movement regardless of button state (use this for 'm' toggle)
	glutMouseWheelFunc(mouse_wheel_callback);   // For mouse wheel events

//	glutCloseFunc(cleanup_OpenGL_stuffs or else); // Do it yourself!!!
}

//where light attributes are defined
void initialize_lights(void) {
	scene.n_lights = 3;

	// Light 0: Static Point Light in the center of the big room
	scene.lights[0].light_on = true;
	scene.lights[0].position = glm::vec4(120.0f, 100.0f, 40.0f, 1.0f); // World Coordinates
	scene.lights[0].ambient_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	scene.lights[0].diffuse_color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	scene.lights[0].specular_color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f); 
	scene.lights[0].spot_direction = glm::vec3(0.0f, 0.0f, -1.0f); // Not used for point light
	scene.lights[0].spot_exponent = 0.0f;
	scene.lights[0].spot_cutoff_angle = 180.0f; // Effectively a point light
	scene.lights[0].light_attenuation_factors = glm::vec4(1.0f, 0.005f, 0.0001f, 0.0f); // Constant, Linear, Quadratic

	// Light 1: Dynamic Spotlight attached to the Wolf
	scene.lights[1].light_on = true;
	scene.lights[1].position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Will be updated every frame
	scene.lights[1].ambient_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	scene.lights[1].diffuse_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);	// white color light
	scene.lights[1].specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	scene.lights[1].spot_direction = glm::vec3(0.0f, 0.0f, -1.0f); // Will be updated every frame
	scene.lights[1].spot_exponent = 40.0f; 
	scene.lights[1].spot_cutoff_angle = 30.0f;
	scene.lights[1].light_attenuation_factors = glm::vec4(1.0f, 0.001f, 0.000001f, 0.0f);

	// Light 2: New spotlight attached to the main camera (Flashlight)
	scene.lights[2].light_on = true;
	scene.lights[2].position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Updated every frame from camera position
	scene.lights[2].ambient_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	scene.lights[2].diffuse_color = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f); // dim blue light
	scene.lights[2].specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	scene.lights[2].spot_direction = glm::vec3(0.0f, 0.0f, -1.0f); // Updated every frame from camera direction
	scene.lights[2].spot_exponent = 40.0f;
	scene.lights[2].spot_cutoff_angle = 15.0f;
	scene.lights[2].light_attenuation_factors = glm::vec4(1.0f, 0.045f, 0.0000075f, 0.0f);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST); // Default state
	 
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);
}

void initialize_renderer(void) {
	register_callbacks();
	initialize_OpenGL();
	scene.initialize();
	initialize_lights();
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

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170/AIE4012 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 3
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170/AIE4120 Our_House_GLSL_V_0.55";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys: W/A/S/D/Q/E (Move), M (Toggle Mouse Look), Mouse (Look), Wheel (Zoom), C/F/D/X (Modes), ESC (Exit)",
											"    - 1, 2, 3 (Change to Static Projection Cameras), 4 (Change to Movable Projection Camera)",
											"    - 5, 6, 7 (Change to Static Orthographic Cameras)"};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);

	// ***** THE FIX: Initialize scene.window members here *****
	// These values should match what you pass to glutInitWindowSize
	scene.window.width = 1200;
	scene.window.height = 800;
	if (scene.window.height == 0) { // Safety check, although 800 is not 0
		scene.window.height = 1;    // Prevent division by zero if height was somehow 0
	}
	scene.window.aspect_ratio = (float)scene.window.width / (float)scene.window.height;
	// ***** END OF FIX *****

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	// Ensure cursor is visible initially if mouse look is off by default
	if (!mouseLookActive) {
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
	else {
		glutSetCursor(GLUT_CURSOR_NONE);
		firstMouse = true; // Prepare for immediate mouse input if starting active
	}

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
