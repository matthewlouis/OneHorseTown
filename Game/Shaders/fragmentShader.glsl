#version 330 core

in vec2 vTexCoord;

uniform vec4      uColor;
uniform sampler2D uTexture;
uniform float	  uFadeOut; //value between 0.0 and 1.0 to determine fade out amount

out vec4 out_Color;

void main()
{
	float fadeOut = 1 - uFadeOut;
    //vec2 texCoord = round( vTexCoord * 127 ) / 127;
    //vec2 texCoord = round( vTexCoord * 50 ) / 50;
    vec2 texCoord = vTexCoord;
    out_Color = uColor * texture( uTexture, texCoord ) * fadeOut;
}