#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 in_normal;

uniform int shading_mode;
uniform vec3 light_dir;
uniform vec3 camera_dir;
uniform float light_intensity;
uniform float kA;
uniform float kD;
uniform float kS;
uniform float N;

uniform mat4 model;
uniform mat4 transform;

out vec3 frag_normal;
out float gouraud_diffuse;
out float gouraud_specular;

// All light modes have been combined into this shader so I
// don't need to use 4 separate shaders for both vert/frag
// Lighting mode:
// 1 = Gouraud
// 2 = Phong
// Else = disabled

void main()
{
    // Gouraud: Calculate lighting in vertex
    // Pass diffuse + specular info to fragment shader
    // Translation is subtracted from normal position
    // (normals should be agnostic to translation, but should still
    // be affected by the model's rotation)

    vec3 translation = (model * vec4(0, 0, 0, 1)).xyz;
    if (shading_mode == 1)
    {
        vec3 normal = normalize((model * vec4(in_normal, 1.0)).xyz - translation);
        float diffuse = max(0, dot(normal, light_dir)) * kD * light_intensity + kA;

        vec3 vR = 2 * dot(normal, light_dir) * normal - light_dir;
        float specular = pow(max(0, dot(vR, camera_dir)), N) * kS * light_intensity;
        specular = min(specular, diffuse);

        gouraud_diffuse = diffuse;
        gouraud_specular = specular;
    }
    else
    {
        gouraud_diffuse = 0.f;
        gouraud_specular = 0.f;
    }

    gl_Position = transform * model * vec4(pos.x, pos.y, pos.z, 1.0);
    frag_normal = (model * vec4(in_normal, 1.0)).xyz - translation;
}