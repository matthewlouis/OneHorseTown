#version 130
//The fragment shader operates on each pixel in a given polygon

in vec4 fragmentColor;

//This is the 3 component float vector that gets outputted to the screen
//for each pixel.
out vec4 color;

uniform float time;

void main() {
    //Just hardcode the color to red
    color = fragmentColor + vec4(1.0 * (cos(time)+1.0) * 0.5,
    							 1.0 * (cos(time + 2.0)+1.0) * 0.5 ,
    							 1.0 * (sin(time)+1.0) * 0.5,
    							 0.0);
}