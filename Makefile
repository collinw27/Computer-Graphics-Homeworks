HW3.exe: HW4.cpp MeshViewer.h
	g++ HW4.cpp -g -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -I../GLFW/include \
	-L../GLEW/lib -I../GLEW/include \
	-I../GLM \
	-o HW4.exe