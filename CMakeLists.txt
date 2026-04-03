cmake_minimum_required(VERSION 3.28)
project(HW LANGUAGES CXX)

add_executable(HW4 HW4.cpp MeshViewer.cpp)
set_target_properties(HW4 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/colli/Documents/School Stuff/CAP 4730/HW4"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "C:/Users/colli/Documents/School Stuff/CAP 4730/HW4"
)

set(GLFW_DIR "C:/Users/colli/Documents/School Stuff/CAP 4730/GLFW")
set(GLEW_DIR "C:/Users/colli/Documents/School Stuff/CAP 4730/GLEW")
set(GLM_DIR "C:/Users/colli/Documents/School Stuff/CAP 4730/GLM")

target_include_directories(HW4 PRIVATE
    ${GLFW_DIR}/include ${GLEW_DIR}/include ${GLM_DIR}
)

link_directories(
    ${GLFW_DIR}/lib
    ${GLEW_DIR}/lib
)

target_link_libraries(HW4 PRIVATE
    ${GLFW_DIR}/lib/libglfw3.a
    ${GLEW_DIR}/lib/glew32.lib
    opengl32
    gdi32
)