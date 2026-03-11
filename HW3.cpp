// template based on material from learnopengl.com
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "MeshViewer.h"
#include <gtc/type_ptr.hpp>

#include <iostream>

// Some code adapted from https://learnopengl.com/Getting-started/Shaders

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    MeshViewer mesh_viewer {};
    mesh_viewer.add_mesh("monkey.vs", "monkey.fs", "monkey.obj", glm::vec3(1, 0, 0));
    mesh_viewer.add_mesh("monkey.vs", "monkey.fs", "cube.obj", glm::vec3(-1, 0, 0));

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // // glew: load all OpenGL function pointers
    glewInit();
    
    // Load multiple meshes

    constexpr int MESH_COUNT = 2;
    GLMesh meshes[2];
    for (int i = 0; i < MESH_COUNT; ++i)
    {
        // build and compile our shader program
        // ------------------------------------
        // vertex shader

        std::string vertexShaderString = mesh_viewer.get_vertex_shader(i);
        std::string fragmentShaderString = mesh_viewer.get_fragment_shader(i);
        const char* vertexShaderSource {vertexShaderString.c_str()};
        const char* fragmentShaderSource {fragmentShaderString.c_str()};

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        std::vector<float> vertexVec = mesh_viewer.get_vertices(i);
        unsigned int numVertices = vertexVec.size() * sizeof(float) / 3;
        
        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        float* vertices = vertexVec.data();
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexVec.size() * sizeof(float), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);

        meshes[i] = GLMesh {
            VAO, VBO,
            shaderProgram,
            numVertices
        };
    }

   
    // start tracking time
    float currentTime = glfwGetTime();
    float lastTime = currentTime;

    // start tracking keys
    int keys[12] = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
        GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_E, GLFW_KEY_R
    };
    bool keyStates[12] = {false, false, false, false, false, false, false,
        false, false, false, false, false
    };
    bool holdingENTER = false;

    // uncomment this call to draw in wireframe polygons.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        
        // process updates
        currentTime = glfwGetTime();
        for (int i = 0; i < 12; i++)
        {
            int keyState = glfwGetKey(window, keys[i]);
            keyStates[i] = keyStates[i] || (keyState == GLFW_PRESS);
            keyStates[i] = keyStates[i] && !(keyState == GLFW_RELEASE);
        }
        UpdateInfo updateInfo {
            currentTime - lastTime,
            keyStates[0], keyStates[1], keyStates[2], keyStates[3],
            keyStates[4], keyStates[5], keyStates[6], keyStates[7],
            keyStates[8], keyStates[9], keyStates[10], keyStates[11],
            !holdingENTER && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS
        };
        holdingENTER = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
        lastTime = currentTime;

        mesh_viewer.update(updateInfo);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render all meshes

        for (int i = 0; i < MESH_COUNT; ++i)
        {
            GLMesh glmesh = meshes[i];

            // draw our first triangle
            glUseProgram(meshes[i].shaderProgram);
            glBindVertexArray(meshes[i].VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            glDrawArrays(GL_TRIANGLES, 0, meshes[i].vertexCount);
            glBindVertexArray(0); // unbind our VA no need to unbind it every time

            // update model matrix
            glm::mat4 model_mat = mesh_viewer.get_model_mat(i);
            auto model_loc = glGetUniformLocation(meshes[i].shaderProgram, "model_mat");
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
            glm::mat4 view_mat = glm::mat4(1.f);
            view_mat = glm::translate(view_mat, glm::vec3(0.f, 0.f, -3.f));
            glUniformMatrix4fv(glGetUniformLocation(meshes[i].shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view_mat));
            glm::mat4 perspective_mat = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
            glUniformMatrix4fv(glGetUniformLocation(meshes[i].shaderProgram, "perspective"), 1, GL_FALSE, glm::value_ptr(perspective_mat));
        }
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}