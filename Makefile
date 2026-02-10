HW2.exe: HW2.cpp RayTracer.h
	g++ HW2.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32 -L../GLFW/lib -L../GLEW/lib -I../GLFW/include -I../GLEW/include -o HW2.exe

debuginfo: HW2.cpp RayTracer.h
	g++ -g HW2.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32 -L../GLFW/lib -L../GLEW/lib -I../GLFW/include -I../GLEW/include -o HW2.exe