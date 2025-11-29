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

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDirection = normalize(-lightDir);
    
    vec3 ambient = vec3(0.35, 0.32, 0.30) * lightColor;
    
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * 1.0;
    
    vec3 lighting = ambient + diffuse;
    vec3 baseColor = fragColor.rgb * lighting;
    
    // THICK FOG with SUBTLE reddish-brown tint
    float fogStart = 15.0;
    float fogEnd = 80.0;
    vec3 fogColor = vec3(0.3, 0.30, 0.3);
    
    float fogFactor = 1.0 - exp(-0.04 * max(0.0, fragDistance - fogStart));
    fogFactor = 0; // clamp(fogFactor, 0.0, 1.0);
    
    vec3 finalColorRGB = mix(baseColor, fogColor, fogFactor);
    
    // Less desaturation
    float gray = dot(finalColorRGB, vec3(0.299, 0.587, 0.114));
    finalColorRGB = mix(finalColorRGB, vec3(gray), 0.15);
    
    // Slight warm reddish tint
    finalColorRGB.r *= 0.95;  // Bit more red
    finalColorRGB.g *= 0.95;  
    finalColorRGB.b *= 0.98;  // Less blue
    
    finalColor = vec4(finalColorRGB, 1.0);
}

