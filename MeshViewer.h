#ifndef MESH_VIEWER_H
#define MESH_VIEWER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtc/type_ptr.hpp>
#include <glm.hpp>

struct UpdateInfo;

struct Mesh
{
    glm::vec3 position {};
    float x_rotation = 0.f;
    float y_rotation = 0.f;
    float scale = 1.0;

    GLuint VAO, VBO = 0;
    GLuint shaderProgram = 0;
    GLuint vertexCount = 0;
};

struct Camera
{
    float distance = 3.f;
    float x_rotation = 0.f;
    float y_rotation = 0.f;
};

class MeshViewer
{
    GLFWwindow* window;
    
    Camera camera {};

    unsigned mesh_count;
    std::vector<Mesh> meshes;
    int current_index = 0;

    glm::vec3 light_dir {1, 0, 0};
    float light_intensity = 1.f;
    float kD = 1.f;
    float kS = 1.f;
    float specN = 1.f;
    int light_mode = 0;

public:

    MeshViewer();

    void init();

    void add_mesh(std::string vs, std::string fs, std::string obj, glm::vec3 start_pos);
    void set_light(glm::vec3 dir, float intensity, float kD, float kS, float N);
    void set_camera(float distance, glm::vec2 rotation);
    void start_render_loop();

    void enable_wireframe();

private:

    void update(const UpdateInfo& info);

    std::string load_shader(std::string filepath);
    GLuint link_shader(std::string vs_path, std::string fs_path);
    std::vector<GLfloat> load_vertices(std::string filepath);
    glm::mat4 get_model_mat(const Mesh& mesh);
    glm::mat4 get_view_mat();
    glm::mat4 get_view_rot_mat();
};

#endif