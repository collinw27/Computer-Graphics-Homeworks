#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include "glm.hpp"

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT;
    bool keyLEFT, keyRIGHT, keyUP, keyDOWN, keyE, keyR;
    bool keyENTER, keyZ;
};

struct Mesh
{
    std::string vs;
    std::string fs;
    std::vector<float> verts;

    glm::vec3 position;
    float x_rotation = 0.f;
    float y_rotation = 0.f;
    float scale = 1.0;
};

struct GLMesh
{
    unsigned VAO, VBO;
    unsigned shaderProgram;
    unsigned vertexCount;
    std::vector<float> vertexVec;
};

class MeshViewer
{
    unsigned mesh_count;
    std::vector<Mesh> meshes;
    int current_index = 0;
    float elapsed = 0.f;

    bool spinning = false;
    glm::vec3 p1, p2;

public:

    MeshViewer();
    void add_mesh(std::string vs, std::string fs, std::string obj, glm::vec3 start_pos);
    void update(const UpdateInfo& info);

    std::string get_vertex_shader(unsigned mesh_index);
    std::string get_fragment_shader(unsigned mesh_index);
    std::vector<float> get_vertices(unsigned mesh_index);
    glm::mat4 get_model_mat(unsigned mesh_index);

private:

    std::string load_shader(std::string filepath);
    std::vector<float> load_vertices(std::string filepath);
};

MeshViewer::MeshViewer() : meshes{} {}

void MeshViewer::add_mesh(std::string vs, std::string fs, std::string obj, glm::vec3 start_pos)
{
    meshes.push_back(Mesh{load_shader(vs), load_shader(fs), load_vertices(obj), start_pos});
}

void MeshViewer::update(const UpdateInfo& info)
{
    if (spinning)
        elapsed += info.deltaTime;

    // Transform mesh using inputs

    Mesh& current = meshes.at(current_index);
    glm::vec3 delta {};
    if (info.keyW)
        delta = delta + glm::vec3(0.0, 0.0, -1.0);
    if (info.keyA)
        delta = delta + glm::vec3(-1.0, 0.0, 0.0);
    if (info.keyS)
        delta = delta + glm::vec3(0.0, 0.0, 1.0);
    if (info.keyD)
        delta = delta + glm::vec3(1.0, 0.0, 0.0);
    if (info.keySHIFT)
        delta = delta + glm::vec3(0.0, -1.0, 0.0);
    if (info.keySPACE)
        delta = delta + glm::vec3(0.0, 1.0, 0.0);
    if (info.keyLEFT)
        current.x_rotation += info.deltaTime * 2;
    if (info.keyRIGHT)
        current.x_rotation -= info.deltaTime * 2;
    if (info.keyUP)
        current.y_rotation -= info.deltaTime * 2;
    if (info.keyDOWN)
        current.y_rotation += info.deltaTime * 2;
    if (info.keyE)
        current.scale = std::max(current.scale - info.deltaTime, 0.2f);
    if (info.keyR)
        current.scale += info.deltaTime;
    if (info.keyZ)
        spinning = !spinning;
    current.position = current.position + delta * info.deltaTime;

    // Switch between meshes

    if (info.keyENTER)
    {
        current_index = (current_index + 1) % meshes.size();
        std::cout << "Selected mesh #" << current_index << std::endl;
    }
}

glm::mat4 MeshViewer::get_model_mat(unsigned mesh_index)
{
    Mesh& mesh = meshes.at(mesh_index);
    auto T = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        mesh.position.x, mesh.position.y, mesh.position.z, 1
    );
    auto R_X = glm::mat4(
        cos(mesh.x_rotation), 0, sin(mesh.x_rotation), 0,
        0, 1, 0, 0,
        -sin(mesh.x_rotation), 0, cos(mesh.x_rotation), 0,
        0, 0, 0, 1
    );
    auto R_Y = glm::mat4(
        1, 0, 0, 0,
        0, cos(mesh.y_rotation), sin(mesh.y_rotation), 0,
        0, -sin(mesh.y_rotation), cos(mesh.y_rotation), 0,
        0, 0, 0, 1
    );
    auto S = glm::mat4(
        mesh.scale, 0, 0, 0,
        0, mesh.scale, 0, 0,
        0, 0, mesh.scale, 0,
        0, 0, 0, 1
    );

    // Spin around central axis if instructed to
    // See report for formulas

    glm::mat4 M;
    {
        glm::vec3 a = p2 - p1;
        float phi = std::atan(std::sqrt(a.x*a.x + a.z*a.z)/a.y);
        float theta = std::atan(a.z/a.x);
        float angle = elapsed * 2;
        auto R_phi = glm::mat4(
            cos(phi), sin(phi), 0, 0,
            -sin(phi), cos(phi), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
        auto R_theta = glm::mat4(
            cos(theta), 0, sin(theta), 0,
            0, 1, 0, 0,
            -sin(theta), 0, cos(theta), 0,
            0, 0, 0, 1
        );
        auto T = glm::mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            p1.x, p1.y, p1.z, 1
        );
        auto R = glm::mat4(
            1, 0, 0, 0,
            0, cos(angle), sin(angle), 0,
            0, -sin(angle), cos(angle), 0,
            0, 0, 0, 1
        );
        M = T * R_theta * R_phi * R * glm::inverse(R_phi) * glm::inverse(R_theta) * glm::inverse(T);
    }

    // Note that mesh rotation doesn't affect translation direction

    return M * S * T * R_Y * R_X;
}

std::string MeshViewer::get_vertex_shader(unsigned mesh_index) { return meshes.at(mesh_index).vs; }
std::string MeshViewer::get_fragment_shader(unsigned mesh_index) { return meshes.at(mesh_index).fs; }
std::vector<float> MeshViewer::get_vertices(unsigned mesh_index) { return meshes.at(mesh_index).verts; }

std::string MeshViewer::load_shader(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw new std::runtime_error("Could not open file!");
    std::string a = (std::stringstream{} << file.rdbuf()).str();
    return std::string(a);
}

std::vector<float> MeshViewer::load_vertices(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw new std::runtime_error("Could not open file!");
    std::vector<float> vertices;
    std::vector<std::vector<float>> indexed_vertices {};
    std::string buffer;
    while (std::getline(file, buffer))
    {
        std::string buffer2;
        std::vector<std::string> tokens {};
        std::stringstream stream_buffer {buffer};
        while (std::getline(stream_buffer, buffer2, ' '))
            tokens.push_back(buffer2);
        if (tokens.at(0) == "v")
        {
            std::vector<float> vert {};
            for (int i = 1; i <= 3; i++)
                vert.push_back(std::stof(tokens.at(i)));
            indexed_vertices.push_back(vert);
        }
        else if (tokens.at(0) == "f")
        {
            for (int i = 1; i <= 3; i++)
            {
                std::getline(std::stringstream{tokens.at(i)}, buffer2, '/');
                int vert_index = std::stoi(buffer2);
                for (int j = 0; j < 3; j++)
                    vertices.push_back(indexed_vertices.at(vert_index - 1).at(j));
            }
        }
    }
    return vertices;
}