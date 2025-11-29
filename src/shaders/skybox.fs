#version 330

in vec3 fragTexCoord;

uniform samplerCube skybox;

out vec4 finalColor;

void main()
{
    finalColor = texture(skybox, fragTexCoord);
}

