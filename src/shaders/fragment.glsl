// src/shaders/fragment.glsl
#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec4 fragColor;
in float fragDistance;

out vec4 finalColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 worldCenter;
uniform float worldRadius;

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDirection = normalize(-lightDir);
    
    vec3 ambient = vec3(0.35, 0.32, 0.30) * lightColor;
    
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * 1.0;
    
    vec3 lighting = ambient + diffuse;
    vec3 baseColor = fragColor.rgb * lighting;
    
    // Calculate distance from world center (XZ plane only)
    float distFromCenter = length(fragPosition.xz - worldCenter.xz);
    
    // Base fog
    float fogStart = 15.0;
    float fogEnd = 80.0;
    vec3 fogColor = vec3(0.3, 0.3, 0.3);
    
    float baseFogFactor = 0.0; // Default disabled
    
    // Boundary fog - increases beyond world radius
    float boundaryFogFactor = 0.0;
    if (distFromCenter > worldRadius) {
        float excessDist = distFromCenter - worldRadius;
        // Gradually increase fog density as we go beyond boundary
        boundaryFogFactor = clamp(excessDist / 200.0, 0.0, 0.85);
        fogColor = mix(fogColor, vec3(0.2, 0.2, 0.25), boundaryFogFactor * 0.5);
    }
    
    float fogFactor = max(baseFogFactor, boundaryFogFactor);
    
    vec3 finalColorRGB = mix(baseColor, fogColor, fogFactor);
    
    // Less desaturation
    float gray = dot(finalColorRGB, vec3(0.299, 0.587, 0.114));
    finalColorRGB = mix(finalColorRGB, vec3(gray), 0.15);
    
    finalColorRGB.r *= 0.95; 
    finalColorRGB.g *= 0.95;  
    finalColorRGB.b *= 1.0;
    
    finalColor = vec4(finalColorRGB, 1.0);
}

