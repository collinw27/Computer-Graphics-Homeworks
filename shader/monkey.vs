#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
uniform mat4 transform;

out vec3 frag_normal;

void main()
{
    gl_Position = transform * vec4(pos.x, pos.y, pos.z, 1.0);
    frag_normal = normal;
}