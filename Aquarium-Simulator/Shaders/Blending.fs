#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 lightBlueColor = vec4(0.678, 0.847, 0.902, 0.3); // RGB values for light blue
    FragColor = lightBlueColor;
}