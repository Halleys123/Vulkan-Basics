#version 450

layout(location = 0) in vec3 fragColor; // Interpolated color, location must match in both

layout(location = 0) out vec4 outFragColor; // Final output color must have location

void main() {
    outFragColor = vec4(fragColor, 1.0);
}
