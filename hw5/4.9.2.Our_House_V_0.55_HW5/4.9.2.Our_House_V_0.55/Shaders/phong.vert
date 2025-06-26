#version 330

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_tex_coord;

// Matrices
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat4 u_ModelViewMatrixInvTrans;

// To Fragment Shader
out vec3 f_position_EC;
out vec3 f_normal_EC;

void main() {
    // Transform vertex position and normal to Eye Coordinates
    f_position_EC = (u_ModelViewMatrix * vec4(v_position, 1.0)).xyz;
    f_normal_EC = mat3(u_ModelViewMatrixInvTrans) * v_normal;

    gl_Position = u_ModelViewProjectionMatrix * vec4(v_position, 1.0);
}