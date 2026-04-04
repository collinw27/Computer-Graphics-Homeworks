#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 in_normal;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(pos.x, pos.y, pos.z, 1.0);
}