#version 130

in vec3 vertex; //loc 0
in vec2 texCoord; //loc 1

uniform mat4 uMatrix;

out vec2 vTexCoord;

void main()
{
    gl_Position = uMatrix * vec4( vertex, 1 );;
    vTexCoord = texCoord;
}