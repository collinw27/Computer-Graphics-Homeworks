#version 330 core

uniform int lighting_mode;
uniform vec3 light_dir;
uniform float light_intensity;
uniform float kD;
uniform float kS;
uniform float N;
uniform vec3 base_color;

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
    float intensity = max(0, dot(frag_normal, light_dir)) * kD * light_intensity;
    frag_color = vec4(1.f, 1.f, 1.f, 1.f);
    frag_color = vec4(mix(vec3(0, 0, 0), frag_color.rgb, intensity), 1.f);
}