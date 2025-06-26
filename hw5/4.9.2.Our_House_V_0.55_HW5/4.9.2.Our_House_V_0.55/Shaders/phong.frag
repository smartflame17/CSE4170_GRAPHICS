#version 330

#define N_MAX_LIGHTS 4

// From Vertex Shader
in vec3 f_position_EC;
in vec3 f_normal_EC;

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

out vec4 f_color;

void main() {
    vec3 N = normalize(f_normal_EC);
    vec3 V = normalize(-f_position_EC); // View vector

    // Start with emissive and global ambient light
    vec4 final_color = u_material.emissive_color + u_material.ambient_color * u_global_ambient_color;

    for (int i = 0; i < u_n_lights; i++) {
        if (!u_lights[i].light_on) continue;

        vec3 L; // Light vector
        float attenuation = 1.0;

        if (u_lights[i].position.w == 0.0) { // Directional light
            L = normalize(u_lights[i].position.xyz);
        } else { // Point or Spot light
            vec3 light_pos = u_lights[i].position.xyz;
            L = normalize(light_pos - f_position_EC);
            float dist = length(light_pos - f_position_EC);
            attenuation = 1.0 / (u_lights[i].light_attenuation_factors.x +
                                 u_lights[i].light_attenuation_factors.y * dist +
                                 u_lights[i].light_attenuation_factors.z * dist * dist);
        }

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

        final_color += attenuation * spot_effect * (ambient + diffuse + specular);
    }

    f_color = final_color;
}