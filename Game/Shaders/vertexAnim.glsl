#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMatrix;
uniform int uFacingDirection;
uniform int uCurrentFrame; //what frame to draw
uniform int uCurrentAnim; //the current animation to draw
uniform int uMaxFrames; //max number of frames on sprite sheet
uniform int uTotalAnim; //how many animations on sprite sheet (1 row == 1 anim)


out vec2 vTexCoord;

void main()
{
    gl_Position = uMatrix * vec4( vertex, 1 );
	glScalef(1,0,0);
    vTexCoord =  vec2((uCurrentFrame + texCoord.x) * 1 / uMaxFrames,
					  (uCurrentAnim + texCoord.y) * 1/ uTotalAnim);
}