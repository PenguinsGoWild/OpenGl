#version 330 core
in vec3 colour;
in vec4 pos;
out vec4 FragColor;
void main()
{
   //FragColor = vec4(colour, 1.0f);
   FragColor = pos;
}
