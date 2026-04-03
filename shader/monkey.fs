#version 330 core

uniform int lighting_mode;
uniform vec3 light_dir;
uniform vec3 camera_dir;
uniform float light_intensity;
uniform float kD;
uniform float kS;
uniform float N;
uniform vec3 base_color;

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
    vec3 normal = normalize(frag_normal);
    float diffuse = max(0, dot(normal, -light_dir)) * kD * light_intensity;

    vec3 vR = 2 * dot(normal, -light_dir) * normal + light_dir;
    float specular = pow(max(0, dot(vR, camera_dir)), N) * kS * light_intensity;
    specular = min(specular, diffuse);

    frag_color = vec4(0.5f, 0.5f, 1.f, 1.f);
    frag_color = vec4(mix(vec3(0, 0, 0), frag_color.rgb, min(1.f, diffuse)), 1.f);
    frag_color = vec4(mix(frag_color.rgb, vec3(1, 1, 1), min(1.f, specular)), 1.f);
}