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

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT;
    bool keyLEFT, keyRIGHT, keyUP, keyDOWN, keyE, keyR;
    bool keyENTER, keyZ;
};

struct Mesh
{
    std::vector<float> verts {};

    glm::vec3 position {};
    float x_rotation = 0.f;
    float y_rotation = 0.f;
    float scale = 1.0;

    GLuint VAO, VBO = 0;
    GLuint shaderProgram = 0;
    GLuint vertexCount = 0;
};

class MeshViewer
{
    GLFWwindow* window;

    unsigned mesh_count;
    std::vector<Mesh> meshes;
    int current_index = 0;
    float elapsed = 0.f;

    bool spinning = false;
    glm::vec3 p1, p2;

public:

    MeshViewer();

    void init();

    void add_mesh(std::string vs, std::string fs, std::string obj, glm::vec3 start_pos);
    void start_render_loop();

    void enable_wireframe();

private:

    void update(const UpdateInfo& info);

    std::string load_shader(std::string filepath);
    GLuint link_shader(std::string vs_path, std::string fs_path);
    std::vector<float> load_vertices(std::string filepath);
    
    glm::mat4 get_model_mat(const Mesh& mesh);
};

#endif