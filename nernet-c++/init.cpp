#include <glad/gl.h>
#include <GLFW/glfw3.h>

bool startOpenGLContext() {
	int init = glfwInit();

	if (init == GLFW_FALSE) {
		return false;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	const char* windowName = "openglcontext";
	GLFWwindow* window = glfwCreateWindow(1, 1, windowName, NULL, NULL);

	if (window == NULL) {
		return false;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL((GLADloadfunc)glfwGetProcAddress);
	
	return true;
}