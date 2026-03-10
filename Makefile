HW3.exe: HW3.cpp Functions.h
	g++ HW3.cpp -g -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -L../GLEW/lib \
	-I../GLFW/include -I../GLEW/include \
	-o HW3.exe