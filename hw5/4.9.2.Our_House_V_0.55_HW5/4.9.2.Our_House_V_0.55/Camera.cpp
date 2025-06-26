#define _CRT_SECURE_NO_WARNINGS

#include "Camera.h"
#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

// Base constructor for struct Camera
Camera::Camera(Camera_ID _camera_id) : 
	camera_id(_camera_id),
	Front(glm::vec3(0.0f, 0.0f, -1.0f)),	// default front direction
	WorldUp(glm::vec3(0.0f, 0.0f, 1.0f)),	// z-axis is up direction
	Yaw(YAW),
	Pitch(PITCH),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVITY),
	Fov(FOV_INITIAL),
	flag_valid(false),
	flag_move(false){
}

// Perspective_Camera Constructor
Perspective_Camera::Perspective_Camera(Camera_ID _camera_id) : Camera(_camera_id) {}

// Helper to initialize camera attributes from an initial position, target, and world up vector (used in define_camera method)
void Perspective_Camera::initializeAttributes(const glm::vec3& initial_pos, const glm::vec3& look_at_target, const glm::vec3& world_up_vec) {
	cam_view.pos = initial_pos;
	WorldUp = world_up_vec; // Set the world up vector for this camera (will be y-axis)

	// Calculate initial Front vector
	Front = glm::normalize(look_at_target - initial_pos);

	// Calculate initial Yaw and Pitch from the Front vector
	Pitch = glm::degrees(asin(Front.z));
	float cosPitch = cos(glm::radians(Pitch));
	if (abs(cosPitch) < 1e-6) { // If looking straight up or down
		if (Front.z > 0.99f) Yaw = YAW; // Default YAW if looking straight up
		else Yaw = YAW; // Default YAW if looking straight down (or keep previous yaw if interactive)
	}
	else {
		Yaw = glm::degrees(atan2(Front.y, Front.x));
	}

	// Initialize MovementSpeed, MouseSensitivity and Fov from global constants
	MovementSpeed = SPEED;
	MouseSensitivity = SENSITIVITY;
	Fov = FOV_INITIAL;

	updateCameraVectors(); // Calculate Right and Up vectors based on new data
}

void Perspective_Camera::define_camera(int win_width, int win_height, float win_aspect_ratio) {
	// glm::mat3 R33_t;
	// glm::mat4 T;
 
	switch (camera_id) {
	case CAMERA_MAIN: {
		flag_valid = true;
		flag_move = true; // main camera is permitted to move

		glm::vec3 initial_eye = glm::vec3(-120.0f, 20.0f, 30.0f);	// -600.0f, -600.0f, 400.0f originally
		glm::vec3 initial_center = glm::vec3(125.0f, 80.0f, 30.0f);
		glm::vec3 initial_world_up = glm::vec3(0.0f, 0.0f, 1.0f); 

		initializeAttributes(initial_eye, initial_center, initial_world_up);	// now we initialize the main camera from default data given in skeleton

		cam_proj.projection_type = CAMERA_PROJECTION_PERSPECTIVE;
		cam_proj.params.pers.fovy = 15.0f * TO_RADIAN;
		cam_proj.params.pers.aspect = win_aspect_ratio;
		cam_proj.params.pers.n = 1.0f;
		cam_proj.params.pers.f = 50000.0f;

		RecomputeViewMatrix();      // Set initial ViewMatrix
		RecomputeProjectionMatrix(); // Set initial ProjectionMatrix

		view_port.x = 0; view_port.y = 0; view_port.w = win_width; view_port.h = win_height;	//main cam uses full screen as viewport (sub cams will only use parts)

		// view_port.x = 200; view_port.y = 200; view_port.w = win_width - 200; view_port.h = win_height - 200; // original value in skeleton
		break;
	}
	case CAMERA_CC_0: {
		flag_valid = false;
		flag_move = false;

		glm::vec3 initial_eye = glm::vec3(15.0f, 90.0f, 40.0f);		// middle left of building
		glm::vec3 initial_center = glm::vec3(95.0f, 90.0f, 30.0f);	// looking at east corridor
		glm::vec3 initial_world_up = glm::vec3(0.0f, 0.0f, 1.0f);

		initializeAttributes(initial_eye, initial_center, initial_world_up);

		Fov = 60.0f; // A wider FOV for an overview camera

		cam_proj.projection_type = CAMERA_PROJECTION_PERSPECTIVE;
		cam_proj.params.pers.aspect = win_aspect_ratio; // ratio should be 1.0f for sub cams
		cam_proj.params.pers.n = 1.0f;
		cam_proj.params.pers.f = 50000.0f;

		RecomputeViewMatrix();
		RecomputeProjectionMatrix();

		// Viewport is top-left corner, 200x200
		view_port.x = 0; view_port.y = win_height - 300; view_port.w = 300; view_port.h = 300;
		break;
	}
	case CAMERA_CC_1: {
		flag_valid = false;
		flag_move = false;

		glm::vec3 initial_eye = glm::vec3(225.0f, 155.0f, 40.0f);		// top right corner of building
		glm::vec3 initial_center = glm::vec3(210.0f, 85.0f, 20.0f);		// looking at south corridor
		glm::vec3 initial_world_up = glm::vec3(0.0f, 0.0f, 1.0f);

		initializeAttributes(initial_eye, initial_center, initial_world_up);

		Fov = 60.0f;

		cam_proj.projection_type = CAMERA_PROJECTION_PERSPECTIVE;
		cam_proj.params.pers.aspect = win_aspect_ratio;
		cam_proj.params.pers.n = 1.0f;
		cam_proj.params.pers.f = 50000.0f;

		RecomputeViewMatrix();
		RecomputeProjectionMatrix();

		// Viewport is top-left corner, 200x200
		view_port.x = 0; view_port.y = win_height - 300; view_port.w = 300; view_port.h = 300;
		break;
	}
	case CAMERA_CC_2: {
		flag_valid = false;
		flag_move = false;

		glm::vec3 initial_eye = glm::vec3(100.0f, 5.0f, 40.0f);			// middle bottom room corner of building
		glm::vec3 initial_center = glm::vec3(115.0f, 55.0f, 20.0f);		// looking at corridor and room
		glm::vec3 initial_world_up = glm::vec3(0.0f, 0.0f, 1.0f);

		initializeAttributes(initial_eye, initial_center, initial_world_up);

		Fov = 90.0f;	// wide fov

		cam_proj.projection_type = CAMERA_PROJECTION_PERSPECTIVE;
		cam_proj.params.pers.aspect = win_aspect_ratio;
		cam_proj.params.pers.n = 1.0f;
		cam_proj.params.pers.f = 50000.0f;

		RecomputeViewMatrix();
		RecomputeProjectionMatrix();

		// Viewport is top-left corner, 200x200
		view_port.x = 0; view_port.y = win_height - 300; view_port.w = 300; view_port.h = 300;
		break;
	}
	case CAMERA_CC_MOVE: {
		flag_valid = false;
		flag_move = true; 

		glm::vec3 initial_eye = glm::vec3(70.0f, 120.0f, 40.0f);		// corner of middle corridor of building
		glm::vec3 initial_center = glm::vec3(90.0f, 120.0f, 30.0f);
		glm::vec3 initial_world_up = glm::vec3(0.0f, 0.0f, 1.0f);

		initializeAttributes(initial_eye, initial_center, initial_world_up);	

		Fov = 70.0f;	// wide fov

		cam_proj.projection_type = CAMERA_PROJECTION_PERSPECTIVE;
		cam_proj.params.pers.aspect = win_aspect_ratio;
		cam_proj.params.pers.n = 1.0f;
		cam_proj.params.pers.f = 50000.0f;

		RecomputeViewMatrix();     
		RecomputeProjectionMatrix();

		// Viewport is top-left corner, 200x200
		view_port.x = 0; view_port.y = win_height - 300; view_port.w = 300; view_port.h = 300;
		break;
	}
	}
}

// recalculating camera vectors
void Perspective_Camera::updateCameraVectors() {
	// Calculate the new Front vector from Yaw and Pitch
	glm::vec3 temp_front;
	temp_front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	temp_front.y = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	temp_front.z = sin(glm::radians(Pitch));
	Front = glm::normalize(temp_front);

	// Re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Right vector
	Up = glm::normalize(glm::cross(Right, Front));    // Camera's local Up vector
}

// wrapper for lookAt()
void Perspective_Camera::RecomputeViewMatrix() {
	ViewMatrix = glm::lookAt(cam_view.pos, cam_view.pos + Front, Up);
}


// wrapper for perspective()
void Perspective_Camera::RecomputeProjectionMatrix(float aspect_ratio_param) {
	if (aspect_ratio_param > 0) { // If a new aspect ratio is provided (e.g., on reshape)
		cam_proj.params.pers.aspect = aspect_ratio_param;
	}
	cam_proj.params.pers.fovy = glm::radians(Fov); // Use current Fov (degrees)
	ProjectionMatrix = glm::perspective(cam_proj.params.pers.fovy, cam_proj.params.pers.aspect,
		cam_proj.params.pers.n, cam_proj.params.pers.f);
}

// called in keyboard callback
void Perspective_Camera::ProcessKeyboardMovement(unsigned char key, float deltaTime) {
	if (!flag_move) return; // Only process if camera is movable
	float velocity = MovementSpeed * deltaTime;
	switch (key) {
	case 'w': // Forward
		cam_view.pos += Front * velocity;
		break;
	case 's': // Backward
		cam_view.pos -= Front * velocity;
		break;
	case 'a': // Left
		cam_view.pos -= Right * velocity;
		break;
	case 'd': // Right
		cam_view.pos += Right * velocity;
		break;
	case 'q': // Down (along world Z-axis)
		cam_view.pos -= WorldUp * velocity;
		break;
	case 'e': // Up (along world Z-axis)
		cam_view.pos += WorldUp * velocity;
		break;
	}
	RecomputeViewMatrix(); // Update ViewMatrix after position change
}

// called in mouse callback
void Perspective_Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
	if (!flag_move) return;
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw -= xoffset;
	Pitch += yoffset;

	// Constrain pitch to avoid flipping
	if (constrainPitch) {
		if (Pitch > 89.0f) Pitch = 89.0f;
		if (Pitch < -89.0f) Pitch = -89.0f;
	}

	updateCameraVectors(); // Update Front, Right, Up vectors from new Yaw/Pitch
	RecomputeViewMatrix(); // Update ViewMatrix after orientation change
}

// called in mouse callback
void Perspective_Camera::ProcessMouseScroll(float yoffset) {
	if (!flag_move) return;
	
	// Scrolling up zooms in (decreasing FOV)
	Fov -= yoffset * 1.0f; // multiplier is scroll sensitivity

	if (Fov < FOV_MIN) Fov = FOV_MIN;
	if (Fov > FOV_MAX) Fov = FOV_MAX;

	RecomputeProjectionMatrix(); // Update ProjectionMatrix after FOV change
}

// Definitions for Orthographic_Camera
void Orthographic_Camera::define_camera(int win_width, int win_height, float win_aspect_ratio) {
	
	flag_valid = false;
	flag_move = false;

	// Orthographic projection settings
	float ortho_view_extent = 150.0f; // How many units to show from the center (scene scale)
	float ortho_near = -500.0f;      
	float ortho_far = 500.0f;

	cam_proj.projection_type = CAMERA_PROJECTION_ORTHOGRAPHIC;
	cam_proj.params.ortho.left = -ortho_view_extent;
	cam_proj.params.ortho.right = ortho_view_extent;
	cam_proj.params.ortho.bottom = -ortho_view_extent; // "bottom" was misspelled in original Camera.h
	cam_proj.params.ortho.top = ortho_view_extent;
	cam_proj.params.ortho.n = ortho_near;
	cam_proj.params.ortho.f = ortho_far;

	ProjectionMatrix = glm::ortho(cam_proj.params.ortho.left, cam_proj.params.ortho.right,
		cam_proj.params.ortho.bottom, cam_proj.params.ortho.top,
		cam_proj.params.ortho.n, cam_proj.params.ortho.f);

	glm::vec3 scene_center = glm::vec3(120.0f, 80.0f, 25.0f); // center of building
	float camera_distance = 10.0f;  // Distance for ortho cameras from the plane they are viewing
	view_port.x = 0; view_port.y = win_height - 300; view_port.w = 300; view_port.h = 300;

	switch (camera_id) {
	case CAMERA_SIDE_FRONT: // XZ plane view 
		cam_view.pos = scene_center + glm::vec3(0.0f, -camera_distance, 0.0f); // Position in front
		ViewMatrix = glm::lookAt(cam_view.pos,
			scene_center, // Look at scene center in XZ
			glm::vec3(0.0f, 0.0f, 1.0f)); // Z is up in the view
		break;

	case CAMERA_TOP: // XY plane view
		cam_view.pos = scene_center + glm::vec3(0.0f, 0.0f, camera_distance); // Position above
		ViewMatrix = glm::lookAt(cam_view.pos,
			scene_center, // Look at scene center in XY
			glm::vec3(0.0f, 1.0f, 0.0f)); // Y is up in the view
		break;

	case CAMERA_SIDE: // YZ plane view
		cam_view.pos = scene_center + glm::vec3(-camera_distance, 0.0f, 0.0f); // Position to the side
		ViewMatrix = glm::lookAt(cam_view.pos,
			scene_center, // Look at scene center in YZ
			glm::vec3(0.0f, 0.0f, 1.0f)); // Z is up in the view
		break;
	}
}