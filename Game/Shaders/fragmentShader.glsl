#version 130

in vec2 vTexCoord;

uniform vec3      uColor;
uniform sampler2D uTexture;

out vec3 out_Color;

void main()
{
    out_Color = uColor * texture( uTexture, vTexCoord ).rgb;
}