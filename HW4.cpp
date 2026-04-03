// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "MeshViewer.h"
#include <gtc/type_ptr.hpp>

#include <iostream>

// Some code adapted from https://learnopengl.com/Getting-started/Shaders

void processInput(GLFWwindow *window);

// settings

int run()
{
    MeshViewer mesh_viewer {};

    mesh_viewer.init();
    
    // Load mesh/meshes
    
    mesh_viewer.add_mesh("shader/monkey.vs", "shader/monkey.fs", "model/monkey.obj", glm::vec3(0, 0, 0));
    mesh_viewer.add_mesh("shader/monkey.vs", "shader/monkey.fs", "model/cube.obj", glm::vec3(-1, 0, 0));

    // Configure other settings

    mesh_viewer.enable_wireframe();

    // Run program

    mesh_viewer.start_render_loop();

    return 0;
}

int main()
{
    try { return run(); }
    catch (std::runtime_error* err)
    {
        std::cout << err->what() << std::endl;
    }
}

void processInput(GLFWwindow *window)
{
}