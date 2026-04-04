HW4.exe: HW4.cpp MeshViewer.h MeshViewer.cpp
	g++ HW4.cpp MeshViewer.cpp -g -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -I../GLFW/include \
	-L../GLEW/lib -I../GLEW/include \
	-I../GLM \
	-I../STB \
	-o HW4.exe