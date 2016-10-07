#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMatrix;

out vec2 vTexCoord;

void main()
{
    gl_Position = uMatrix * vec4( vertex, 1 );
    vTexCoord = texCoord;
}