// glsl 4.5
#version 450

layout(location = 0) out vec3 fragColor;

// Triangle in a plane, just for sake of satisfaction, we are direclty using these values rather than actually sending data
vec3 vertexPositions[3] = vec3[3](
    vec3(0.0, -0.4, 0.0),
    vec3(0.4, 0.4, 0.0),
    vec3(-0.4, 0.4, 0.0)
);
vec3 vertexColors[3] = vec3[3](
    vec3(0.1, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

// This main can be anything, we need not to use main, becuse vulkan allows to set which is main
void main() {
    gl_Position = vec4(vertexPositions[gl_VertexIndex], 1.0);
    fragColor = vertexColors[gl_VertexIndex];
}
