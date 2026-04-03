#include "MeshViewer.h"

struct UpdateInfo
{
    float deltaTime;
    bool keyW, keyA, keyS, keyD, keySPACE, keySHIFT;
    bool keyLEFT, keyRIGHT, keyUP, keyDOWN, keyE, keyR;
    bool keyENTER, keyZ;
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

MeshViewer::MeshViewer() : meshes{}
{
}

void MeshViewer::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // mesh_viewer.add_mesh("monkey.vs", "monkey.fs", "cube.obj", glm::vec3(-1, 0, 0));

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glewInit();

    glEnable(GL_DEPTH_TEST);
}

void MeshViewer::add_mesh(std::string vs, std::string fs, std::string obj, glm::vec3 start_pos)
{
    // Start constructing mesh object

    Mesh mesh {};

    // Link using helper function (loaded from file)

    mesh.shaderProgram = link_shader(vs, fs);

    std::vector<GLfloat> vertexVec = load_vertices(obj);
    unsigned int numVertices = vertexVec.size() * sizeof(float) / 8;
    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);

    float* vertices = vertexVec.data();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexVec.size() * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Finish setting mesh parameters

    mesh.position = start_pos;
    mesh.VAO = VAO;
    mesh.VBO = VBO;
    mesh.vertexCount = numVertices;

    meshes.push_back(mesh);
}

void MeshViewer::set_light(glm::vec3 dir, float intensity, float kD, float kS, float N)
{
    light_dir = dir;
    light_intensity = intensity;
    this->kD = kD;
    this->kS = kS;
    specN = N;
}

void MeshViewer::start_render_loop()
{
    // Start tracking time

    float currentTime = glfwGetTime();
    float lastTime = currentTime;

    // Start tracking keys

    int keys[12] = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
        GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_E, GLFW_KEY_R
    };
    bool keyStates[12] = {false, false, false, false, false, false, false,
        false, false, false, false, false
    };
    bool holdingENTER = false;
    bool holdingZ = false;

    // Render loop

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        // Process updates

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
            !holdingENTER && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS,
            !holdingZ && glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS
        };
        holdingENTER = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
        holdingZ = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
        lastTime = currentTime;

        update(updateInfo);

        // Render

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render all meshes

        for (Mesh mesh : meshes)
        {
            glUseProgram(mesh.shaderProgram);
            glBindVertexArray(mesh.VAO);

            // Update model matrix

            glm::mat4 model_mat = get_model_mat(mesh);
            glm::mat4 view_mat = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.f));
            glm::mat4 perspective_mat = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
            glm::mat4 transform_mat = perspective_mat * view_mat * model_mat;
            glUniformMatrix4fv(glGetUniformLocation(mesh.shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(transform_mat));

            // Pass in lighting information

            GLint light_mode = 0;
            glUniform1i(glGetUniformLocation(mesh.shaderProgram, "lighting_mode"), light_mode);
            glUniform3fv(glGetUniformLocation(mesh.shaderProgram, "light_dir"), 1, glm::value_ptr(light_dir));
            glUniform1f(glGetUniformLocation(mesh.shaderProgram, "light_intensity"), light_intensity);
            glUniform1f(glGetUniformLocation(mesh.shaderProgram, "kD"), kD);
            glUniform1f(glGetUniformLocation(mesh.shaderProgram, "kS"), kS);
            glUniform1f(glGetUniformLocation(mesh.shaderProgram, "N"), specN);
            
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
            glBindVertexArray(0);
        }
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
}

void MeshViewer::enable_wireframe()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void MeshViewer::update(const UpdateInfo& info)
{
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
    current.position = current.position + delta * info.deltaTime;

    // Switch between meshes

    if (info.keyENTER)
    {
        current_index = (current_index + 1) % meshes.size();
        std::cout << "Selected mesh #" << current_index << std::endl;
    }
}

glm::mat4 MeshViewer::get_model_mat(const Mesh& mesh)
{
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

    // Note that mesh rotation doesn't affect translation direction

    return S * T * R_Y * R_X;
}

std::string MeshViewer::load_shader(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw std::runtime_error("Could not open file!");
    std::string a = (std::stringstream{} << file.rdbuf()).str();
    return std::string(a);
}

GLuint MeshViewer::link_shader(std::string vs_path, std::string fs_path)
{
    std::string vertexShaderString = load_shader(vs_path);
    std::string fragmentShaderString = load_shader(fs_path);
    const char* vertexShaderSource {vertexShaderString.c_str()};
    const char* fragmentShaderSource {fragmentShaderString.c_str()};

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

std::vector<GLfloat> MeshViewer::load_vertices(std::string filepath)
{
    std::ifstream file{filepath};
    if (!file.is_open())
        throw std::runtime_error("Could not open file!");
    std::vector<float> vertices;
    std::vector<std::vector<float>> indexed_positions {};
    std::vector<std::vector<float>> indexed_uvs {};
    std::vector<std::vector<float>> indexed_normals {};
    std::string buffer;
    while (std::getline(file, buffer))
    {
        std::string buffer2;
        std::vector<std::string> tokens {};
        std::stringstream stream_buffer {buffer};
        while (std::getline(stream_buffer, buffer2, ' '))
            tokens.push_back(buffer2);

        // Start by reading vertex data
        // Stored in 3 separate tables:
        // v = vertex pos, vt = UV pos, vn = vertex normal

        if (tokens.at(0) == "v")
        {
            std::vector<float> vert {};
            for (int i = 1; i <= 3; i++)
                vert.push_back(std::stof(tokens.at(i)));
            indexed_positions.push_back(vert);
        }
        else if (tokens.at(0) == "vt")
        {
            std::vector<float> vert {};
            for (int i = 1; i <= 2; i++)
                vert.push_back(std::stof(tokens.at(i)));
            indexed_uvs.push_back(vert);
        }
        else if (tokens.at(0) == "vn")
        {
            std::vector<float> vert {};
            for (int i = 1; i <= 3; i++)
                vert.push_back(std::stof(tokens.at(i)));
            indexed_normals.push_back(vert);
        }

        // Each face combines data for these 3 components
        // Assumed that all face data is after vertex data

        else if (tokens.at(0) == "f")
        {
            for (int i = 1; i <= 3; i++)
            {
                std::stringstream tokenStream {tokens.at(i)};
                std::getline(tokenStream, buffer2, '/');
                int vert_index = std::stoi(buffer2);
                for (int j = 0; j < 3; j++)
                    vertices.push_back(indexed_positions.at(vert_index - 1).at(j));
                std::getline(tokenStream, buffer2, '/');
                vert_index = std::stoi(buffer2);
                for (int j = 0; j < 2; j++)
                    vertices.push_back(indexed_uvs.at(vert_index - 1).at(j));
                std::getline(tokenStream, buffer2);
                vert_index = std::stoi(buffer2);
                for (int j = 0; j < 3; j++)
                    vertices.push_back(indexed_normals.at(vert_index - 1).at(j));
            }
        }
    }
    return vertices;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}