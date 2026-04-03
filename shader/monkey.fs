#version 330 core

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
    frag_color = vec4(abs(frag_normal.x), abs(frag_normal.y), abs(frag_normal.z), 1.0f);
}