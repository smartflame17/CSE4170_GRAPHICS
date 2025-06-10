#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Projection_TYPE {
	CAMERA_PROJECTION_PERSPECTIVE = 0, CAMERA_PROJECTION_ORTHOGRAPHIC
};

enum Camera_ID {
	CAMERA_MAIN = 0,								// main cam (user-controlled)
	CAMERA_SIDE_FRONT, CAMERA_TOP, CAMERA_SIDE,		// orthographic cams
	CAMERA_CC_0, CAMERA_CC_1, CAMERA_CC_2,			// rooted CCTVs
	CAMERA_CC_MOVE									// movable CCTV (fov and direction)
};

// Enum for keyboard movement
enum Camera_Movement {
	FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
};

// Default camera values - these can be tuned
const float YAW = -135.0f; // Adjusted based on initial lookAt: from (-600,-600) to (125,80) in XY plane
const float PITCH = -15.0f;  // Adjusted based on initial lookAt: from z=400 to z=25
const float SPEED = 250.0f;		// Movement speed, units per second
const float SENSITIVITY = 0.1f;  // Mouse sensitivity
const float FOV_INITIAL = 20.0f; // Initial Field of View in degrees
const float FOV_MIN = 1.0f;		// Minimum zoom fov
const float FOV_MAX = 45.0f;	// Maximum zoom fov

struct Camera_View {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
};
 
struct Camera_Projection {
	Camera_Projection_TYPE projection_type;
	union {
		struct {
			float fovy, aspect, n, f;
		} pers;
		struct {
			float left, right, bottom, top, n, f;
		} ortho;
	} params;
};

struct View_Port {
	float x, y, w, h;
};

struct Camera {
	Camera_ID camera_id;
	Camera_View cam_view;
	glm::mat4 ViewMatrix;
	Camera_Projection cam_proj;
	glm::mat4 ProjectionMatrix;
	View_Port view_port;

	//added attributes for moving camera
	glm::vec3 Front;		// Direction camera is looking at
	glm::vec3 Up;           // Camera's local up vector
	glm::vec3 Right;        // Camera's local right vector
	glm::vec3 WorldUp;      // Up direction in world space (e.g., Z-axis)

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Fov;

	bool flag_valid;	// Is camera valid (displayed on screen)
	bool flag_move;		// Is camera movable (with user input)

	Camera(Camera_ID _camera_id);

	// virtual methods to be implemented by derived classes like Perspective_Camera
	virtual void updateCameraVectors() = 0;
	virtual void ProcessKeyboardMovement(unsigned char key, float deltaTime) = 0;
	virtual void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) = 0;
	virtual void ProcessMouseScroll(float yoffset) = 0;
	virtual void RecomputeViewMatrix() = 0;
	virtual void RecomputeProjectionMatrix(float aspect_ratio_param = -1.0f) = 0; // Allow aspect ratio update
};

struct Perspective_Camera : public Camera {
	Perspective_Camera(Camera_ID _camera_id);
	void define_camera(int win_width, int win_height, float win_aspect_ratio);

	// Implemented virtual methods
	void updateCameraVectors() override;
	void ProcessKeyboardMovement(unsigned char key, float deltaTime) override;
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) override;
	void ProcessMouseScroll(float yoffset) override;
	void RecomputeViewMatrix() override;
	void RecomputeProjectionMatrix(float aspect_ratio_param = -1.0f) override;

private:
	void initializeAttributes(const glm::vec3& initial_pos, const glm::vec3& look_at_target, const glm::vec3& world_up_vec);
};

struct Orthographic_Camera : public Camera {
	Orthographic_Camera(Camera_ID _camera_id) : Camera(_camera_id) {}
	void define_camera(int win_width, int win_height, float win_aspect_ratio);

	// Stub implementations for Orthographic camera if it's not controlled
	void updateCameraVectors() override { /* Not typically FPS controlled */ }
	void ProcessKeyboardMovement(unsigned char, float) override { /* Stub */ }
	void ProcessMouseMovement(float, float, bool) override { /* Stub */ }
	void ProcessMouseScroll(float) override { /* Stub */ }
	void RecomputeViewMatrix() override { /* Needs specific logic if movable */ }
	void RecomputeProjectionMatrix(float = -1.0f) override { /* Needs specific logic if zoomable/resizable */ }
};

struct Camera_Data {
	Perspective_Camera cam_main { CAMERA_MAIN };
	Perspective_Camera cam_cc_0 { CAMERA_CC_0 };
	Perspective_Camera cam_cc_1 { CAMERA_CC_1 };
	Perspective_Camera cam_cc_2 { CAMERA_CC_2 };
	Perspective_Camera cam_cc_move{ CAMERA_CC_MOVE };
	Orthographic_Camera cam_front{ CAMERA_SIDE_FRONT };
	Orthographic_Camera cam_top{ CAMERA_TOP };
	Orthographic_Camera cam_side{ CAMERA_SIDE };
};