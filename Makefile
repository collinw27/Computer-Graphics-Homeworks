HW3.exe: HW3.cpp MeshViewer.h
	g++ HW3.cpp -g -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -I../GLFW/include \
	-L../GLEW/lib -I../GLEW/include \
	-I../GLM \
	-o HW3.exe

TriangleExample: TriangleExample.cpp
	g++ TriangleExample.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -L../GLEW/lib \
	-I../GLFW/include -I../GLEW/include \
	-o HW3.exe