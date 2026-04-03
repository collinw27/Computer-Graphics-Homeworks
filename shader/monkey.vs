#version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 model_mat;
uniform mat4 view;
uniform mat4 perspective;

void main()
{
   gl_Position = perspective * view * model_mat * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}