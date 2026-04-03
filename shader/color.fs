#version 330 core

in vec3 frag_normal;
in float gouraud_diffuse;
in float gouraud_specular;

uniform int shading_mode;
uniform vec3 light_dir;
uniform vec3 camera_dir;
uniform float light_intensity;
uniform float kA;
uniform float kD;
uniform float kS;
uniform float N;
uniform vec3 base_color;

out vec4 frag_color;

void main()
{
    frag_color = vec4(0.5f, 0.5f, 1.f, 1.f);

    if (shading_mode == 1 || shading_mode == 2)
    {
        float diffuse = gouraud_diffuse;
        float specular = gouraud_specular;

        if (shading_mode == 2)
        {
            vec3 normal = normalize(frag_normal);
            diffuse = max(0, dot(normal, light_dir)) * kD * light_intensity + kA;

            vec3 vR = 2 * dot(normal, light_dir) * normal - light_dir;
            specular = pow(max(0, dot(vR, camera_dir)), N) * kS * light_intensity;
            specular = min(specular, diffuse);
        }
        frag_color = vec4(mix(vec3(0, 0, 0), frag_color.rgb, min(1.f, diffuse)), 1.f);
        frag_color = vec4(mix(frag_color.rgb, vec3(1, 1, 1), min(1.f, specular)), 1.f);
    }
}