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
    bool keyLEFT, keyRIGHT, keyUP, keyDOWN;
    bool keyENTER, keyE, keyR, keyZ;
};

class MeshViewer
{
    std::string vs;
    std::string fs;
    std::vector<float> verts;

    glm::vec3 translation;
    float x_rotation = 0.f;
    float y_rotation = 0.f;
    float scale = 1.0;

public:

    MeshViewer(std::string vs, std::string fs, std::string obj);
    void update(const UpdateInfo& info);
    glm::mat4 get_model_mat();

    std::string vertex_shader();
    std::string fragment_shader();
    std::vector<float> vertices();

private:

    std::string load_shader(std::string filepath);
    std::vector<float> load_vertices(std::string filepath);
};

MeshViewer::MeshViewer(std::string vs, std::string fs, std::string obj)
{
    this->vs = load_shader(vs);
    this->fs = load_shader(fs);
    this->verts = load_vertices(obj);
    translation = glm::vec3();
}

void MeshViewer::update(const UpdateInfo& info)
{
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
        x_rotation += info.deltaTime;
    if (info.keyRIGHT)
        x_rotation -= info.deltaTime;
    if (info.keyUP)
        y_rotation -= info.deltaTime;
    if (info.keyDOWN)
        y_rotation += info.deltaTime;
    if (info.keyE)
        scale = std::max(scale - info.deltaTime, 0.2f);
    if (info.keyR)
        scale += info.deltaTime;
    translation = translation + delta * info.deltaTime;
}

glm::mat4 MeshViewer::get_model_mat()
{
    auto T = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        translation.x, translation.y, translation.z, 1
    );
    auto R_X = glm::mat4(
        cos(x_rotation), 0, sin(x_rotation), 0,
        0, 1, 0, 0,
        -sin(x_rotation), 0, cos(x_rotation), 0,
        0, 0, 0, 1
    );
    auto R_Y = glm::mat4(
        1, 0, 0, 0,
        0, cos(y_rotation), sin(y_rotation), 0,
        0, -sin(y_rotation), cos(y_rotation), 0,
        0, 0, 0, 1
    );
    auto S = glm::mat4(
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, scale, 0,
        0, 0, 0, 1
    );
    return S * R_Y * R_X * T;
}

std::string MeshViewer::vertex_shader() { return vs; }
std::string MeshViewer::fragment_shader() { return fs; }
std::vector<float> MeshViewer::vertices() { return verts; }

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