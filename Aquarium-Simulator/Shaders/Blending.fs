#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
   // FragColor = texture(texture1, TexCoords);
   // Get the original texture color
    vec4 originalColor = texture(texture1, TexCoords);
    
    // Define the light blue color
    vec4 lightBlueColor = vec4(0.678, 0.847, 0.902, 1.0); // RGB values for light blue
    
    // Blend the texture color with the light blue color
    FragColor = mix(originalColor, lightBlueColor, 0.5); // Adjust the blend factor (0.5 here) as needed
}