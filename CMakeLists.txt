cmake_minimum_required(VERSION 3.28)
project(HW LANGUAGES CXX)

add_executable(HW4 HW4.cpp MeshViewer.cpp)
set_target_properties(HW4 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.."
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/.."
)

set(GLFW_DIR "${CMAKE_BINARY_DIR}/../../GLFW")
set(GLEW_DIR "${CMAKE_BINARY_DIR}/../../GLEW")
set(GLM_DIR "${CMAKE_BINARY_DIR}/../../GLM")
set(STB_DIR "${CMAKE_BINARY_DIR}/../../STB")

target_include_directories(HW4 PRIVATE
    ${GLFW_DIR}/include ${GLEW_DIR}/include ${GLM_DIR} ${STB_DIR}
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