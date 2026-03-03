#version 330 core

// Input attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// Uniforms
uniform mat4 viewProj;
uniform mat4 model;

// Outputs to fragment shader
out vec3 fragPosition;  // Position in world space
out vec3 fragNormal;    // Normal in world space

void main() {
    // Transform position to world space
    vec4 worldPos = model * vec4(position, 1.0);
    fragPosition = worldPos.xyz;
    
    // Transform normal to world space (using transpose of inverse for non-uniform scaling)
    fragNormal = mat3(transpose(inverse(model))) * normal;
    
    // Transform to clip space for rendering
    gl_Position = viewProj * worldPos;
}