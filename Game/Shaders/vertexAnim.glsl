#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMatrix;
uniform int uFacingDirection;

uniform float uCurrentFrame; //what frame to draw
uniform float uCurrentAnim; //the current animation to draw
uniform float uMaxFrames; //max number of frames on sprite sheet
uniform float uTotalAnim; //how many animations on sprite sheet (1 row == 1 anim)


out vec2 vTexCoord;

void main()
{
	vec3 directedVertex = vec3(vertex.x * uFacingDirection, vertex.y, vertex.z);
    gl_Position = uMatrix * vec4( directedVertex, 1 );
    vTexCoord =  vec2( (uCurrentFrame + texCoord.x) / uMaxFrames,
					   (uCurrentAnim + texCoord.y) / uTotalAnim );
    //vTexCoord = texCoord;
}