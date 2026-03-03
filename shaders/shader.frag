#version 330 core

// Inputs from vertex shader
in vec3 fragPosition;
in vec3 fragNormal;

// Uniforms
uniform vec3 DiffuseColor;

// Output color
out vec4 fragColor;

void main() {
    // Normalize the interpolated normal
    vec3 norm = normalize(fragNormal);
    
    // === LIGHT 1: White light from the top ===
    vec3 light1Direction = normalize(vec3(0.0, -1.0, 0.0));  // Pointing down
    vec3 light1Color = vec3(1.0, 1.0, 1.0);                  // White light
    float light1Intensity = 0.6;
    
    // Calculate diffuse lighting for light 1
    float diff1 = max(dot(norm, -light1Direction), 0.0);
    vec3 diffuse1 = light1Intensity * diff1 * light1Color;
    
    // === LIGHT 2: White light from the front ===
    vec3 light2Direction = normalize(vec3(0.0, 0.0, 1.0));   // Pointing forward
    vec3 light2Color = vec3(1.0, 0.0, 0.0);                  // White light
    float light2Intensity = 0.5;
    
    // Calculate diffuse lighting for light 2
    float diff2 = max(dot(norm, -light2Direction), 0.0);
    vec3 diffuse2 = light2Intensity * diff2 * light2Color;
    
    // === Ambient light (so dark areas aren't completely black) ===
    vec3 ambient = vec3(0.3, 0.3, 0.3);
    
    // Combine all lighting components
    vec3 lighting = ambient + diffuse1 + diffuse2;
    
    // Apply lighting to the surface color
    vec3 finalColor = DiffuseColor * lighting;
    
    // Output final color
    fragColor = vec4(finalColor, 1.0);
}