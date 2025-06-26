#version 330

#define N_MAX_LIGHTS 4

// Input vertex attributes
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_tex_coord;

// --- Uniforms ---
// All lighting and material uniforms are needed here for the calculation.

// Matrices
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat4 u_ModelViewMatrixInvTrans;

// Material Properties
struct Material {
    vec4 emissive_color;
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
    float specular_exponent;
};
uniform Material u_material;

// Light Properties
struct Light {
    bool light_on;
    vec4 position; // In EC
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
    vec3 spot_direction; // In EC
    float spot_exponent;
    float spot_cutoff_angle;
    vec4 light_attenuation_factors;
};
uniform Light u_lights[N_MAX_LIGHTS];
uniform int u_n_lights;

// Global Ambient Color
uniform vec4 u_global_ambient_color = vec4(0.5, 0.5, 0.5, 1.0);


// --- Output to Fragment Shader ---
// The calculated color for this vertex. It will be interpolated across the triangle.
out vec4 f_color;

void main() {
    // 1. Transform vertex position and normal to Eye Coordinates (EC)
    vec3 position_EC = (u_ModelViewMatrix * vec4(v_position, 1.0)).xyz;
    vec3 normal_EC = mat3(u_ModelViewMatrixInvTrans) * v_normal;

    // 2. Define lighting vectors for this vertex
    vec3 N = normalize(normal_EC);
    vec3 V = normalize(-position_EC); // View vector

    // 3. Start with emissive and global ambient light
    vec4 vertex_color = u_material.emissive_color + u_material.ambient_color * u_global_ambient_color;

    // 4. Loop through all lights and add their contributions
    for (int i = 0; i < u_n_lights; i++) {
        if (!u_lights[i].light_on) continue;

        vec3 L; // Light vector
        float attenuation = 1.0;

        // Determine light vector and attenuation
        if (u_lights[i].position.w == 0.0) { // Directional light
            L = normalize(u_lights[i].position.xyz);
        } else { // Point or Spot light
            vec3 light_pos = u_lights[i].position.xyz;
            L = normalize(light_pos - position_EC);
            float dist = length(light_pos - position_EC);
            attenuation = 1.0 / (u_lights[i].light_attenuation_factors.x +
                                 u_lights[i].light_attenuation_factors.y * dist +
                                 u_lights[i].light_attenuation_factors.z * dist * dist);
        }

        // Calculate spotlight effect
        float spot_effect = 1.0;
        if (u_lights[i].spot_cutoff_angle < 180.0) {
            vec3 D = normalize(u_lights[i].spot_direction);
            float cos_cur_angle = dot(-L, D);
            float cos_cutoff_angle = cos(radians(u_lights[i].spot_cutoff_angle));

            if (cos_cur_angle < cos_cutoff_angle) {
                spot_effect = 0.0; // Outside the cone
            } else {
                spot_effect = pow(cos_cur_angle, u_lights[i].spot_exponent);
            }
        }

        // Ambient Component
        vec4 ambient = u_material.ambient_color * u_lights[i].ambient_color;

        // Diffuse Component
        float N_dot_L = max(dot(N, L), 0.0);
        vec4 diffuse = u_material.diffuse_color * u_lights[i].diffuse_color * N_dot_L;

        // Specular Component
        vec3 H = normalize(L + V); // Halfway vector
        float N_dot_H = max(dot(N, H), 0.0);
        vec4 specular = vec4(0.0);
        if (N_dot_L > 0.0) { // Only calculate specular if light is visible
             specular = u_material.specular_color * u_lights[i].specular_color * pow(N_dot_H, u_material.specular_exponent);
        }

        // Add this light's contribution to the final vertex color
        vertex_color += attenuation * spot_effect * (ambient + diffuse + specular);
    }

    // 5. Set the final outputs
    // Pass the final computed color for this vertex to the fragment shader
    f_color = vertex_color;

    // Project the vertex position into clip space
    gl_Position = u_ModelViewProjectionMatrix * vec4(v_position, 1.0);
}
