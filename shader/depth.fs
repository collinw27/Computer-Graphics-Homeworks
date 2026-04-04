#version 330 core

out vec4 frag_color;
float near = 0.1f;
float far = 10.f;

void main()
{
    float z = gl_FragCoord.z;
    float linear_z = -(near * far)/((z * far) - (near + far));
    frag_color = vec4(vec3(linear_z / far), 1.f);
}