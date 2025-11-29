#version 330

in vec3 vertexPosition;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragTexCoord;

void main()
{
    fragTexCoord = vertexPosition;
    
    mat4 rotView = mat4(mat3(view));
    vec4 pos = projection * rotView * vec4(vertexPosition, 1.0);
    
    gl_Position = pos.xyww;
}

