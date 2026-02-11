HW2.exe: HW2.cpp RayTracer.h
	g++ HW2.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32 \
	-L../GLFW/lib -L../GLEW/lib \
	-I../GLFW/include -I../GLEW/include \
	-o HW2.exe

sfml_link: HW2.cpp RayTracer.h
	g++ -std=c++17 HW2.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32 -lsfml-graphics-d -lsfml-window-d -lsfml-system-d \
	-L../GLFW/lib -L../GLEW/lib -L../SFML/lib \
	-I../GLFW/include -I../GLEW/include -I../SFML/include \
	-o HW2.exe