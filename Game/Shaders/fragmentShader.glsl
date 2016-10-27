#version 330 core

in vec2 vTexCoord;

uniform vec4      uColor;
uniform sampler2D uTexture;

out vec4 out_Color;

void main()
{
    //vec2 texCoord = round( vTexCoord * 127 ) / 127;
    //vec2 texCoord = round( vTexCoord * 50 ) / 50;
    vec2 texCoord = vTexCoord;
    out_Color = uColor * texture( uTexture, texCoord );
}