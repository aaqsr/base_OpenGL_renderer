#version 410 core

out vec4 FragColour;

in vec2 TexCoord;

uniform sampler2D theTexture;

void main()
{
    FragColour = texture(theTexture, TexCoord);
}
