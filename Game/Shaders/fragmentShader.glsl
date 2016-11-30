#version 330 core

in vec2 vTexCoord;

uniform vec4      uColor;
uniform sampler2D uTexture;
uniform float	  uFadeOut; //value between 0.0 and 1.0 to determine fade out amount
uniform float	  uSilhoutte;
uniform bool	  uInteractive;

out vec4 out_Color;

void main()
{
	float fadeOut = 1 - uFadeOut;

    vec2 texCoord = vTexCoord;
    out_Color = uColor * texture( uTexture, texCoord ) * fadeOut;

    //silhoutte any sprites that aren't part of the interactive game space
    if(uInteractive)
    	out_Color = vec4(out_Color.r * uSilhoutte, out_Color.g * uSilhoutte, out_Color.b * uSilhoutte, out_Color.a);
}