#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include "glm.hpp"

std::string load_shader(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw new std::runtime_error("Could not open file!");
    std::string a = (std::stringstream{} << file.rdbuf()).str();
    return a;
}

std::vector<float> load_vertices(std::string filepath)
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


class Vec3
{
public:

    float x, y, z;

    Vec3() : x{0}, y{0}, z{0} {}
    Vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}

    Vec3 operator-()
    {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator+(const Vec3& other)
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other)
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar)
    {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator*(const Vec3& other)
    {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }

    float dot(const Vec3& other)
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other)
    {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized()
    {
        float l = length();
        return Vec3(x / l, y / l, z / l);
    }

    std::string str()
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT;
};

class MeshViewer
{
    Vec3 translation;

public:

    void update(const UpdateInfo& info);
    glm::mat4 get_model_mat();
};

void MeshViewer::update(const UpdateInfo& info)
{
    Vec3 delta {};
    if (info.keyW)
        delta = delta + Vec3(0.0, 0.0, -1.0);
    if (info.keyA)
        delta = delta + Vec3(-1.0, 0.0, 0.0);
    if (info.keyS)
        delta = delta + Vec3(0.0, 0.0, 1.0);
    if (info.keyD)
        delta = delta + Vec3(1.0, 0.0, 0.0);
    if (info.keySHIFT)
        delta = delta + Vec3(0.0, -1.0, 0.0);
    if (info.keySPACE)
        delta = delta + Vec3(0.0, 1.0, 0.0);
    translation = translation + delta * info.deltaTime;
    std::cout << translation.str() << std::endl;
}

glm::mat4 MeshViewer::get_model_mat()
{
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        translation.x, translation.y, translation.z, 1
    );
}