#version 330

// Input from the vertex shader, interpolated across the triangle face
in vec4 f_color;

// Final output color of the fragment
out vec4 final_color;

void main() {
    final_color = f_color;
}
