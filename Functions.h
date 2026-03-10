#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

std::string loadShader(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw new std::runtime_error("Could not open file!");
    std::string a = (std::stringstream{} << file.rdbuf()).str();
    return a;
}