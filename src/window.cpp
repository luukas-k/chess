#include "window.h"

#include <iostream>

window::window() 
	:
	_handle(nullptr)
{
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	_handle = glfwCreateWindow(1280, 1280, "Chess", nullptr, nullptr);
	glfwMakeContextCurrent(_handle);
#ifndef __EMSCRIPTEN__
	gladLoadGL((GLADloadfunc)glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		std::cout << message << std::endl;
	}, nullptr);
#endif

	glfwSetWindowUserPointer(_handle, this);
	glfwSetKeyCallback(_handle, [](GLFWwindow *wnd, int key, int scancode, int action, int mods) {
		window *w = reinterpret_cast<window *>(glfwGetWindowUserPointer(wnd));
		w->_input.key_state[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
		if (action == GLFW_PRESS)
			w->_input.key_press[key] = true;
		if (action == GLFW_RELEASE)
			w->_input.key_release[key] = true;
		std::cout << w->_input.key_state[key] << " " << w->_input.key_press[key] << " " << w->_input.key_release[key] << "\n";
	});
	glfwSetMouseButtonCallback(_handle, [](GLFWwindow *wnd, int button, int action, int mods) {
		window *w = reinterpret_cast<window *>(glfwGetWindowUserPointer(wnd));
		w->_input.btn_state[button] = (action == GLFW_PRESS || action == GLFW_REPEAT);
		if (action == GLFW_PRESS)
			w->_input.btn_press[button] = true;
		if (action == GLFW_RELEASE)
			w->_input.btn_release[button] = true;
		std::cout << w->_input.btn_state[button] << " " << w->_input.btn_press[button] << " " << w->_input.btn_release[button] << "\n";
	});
	glfwSetCursorPosCallback(_handle, [](GLFWwindow *wnd, double xpos, double ypos) {
		window *w = reinterpret_cast<window *>(glfwGetWindowUserPointer(wnd));
		w->_input.x = xpos;
		w->_input.y = ypos;
	});
}

window::~window() {
}

int window::get_key(int vk) {
	return glfwGetKey(_handle, vk);
}

int window::get_button(int vk) {
	return glfwGetMouseButton(_handle, vk);
}

std::pair<double, double> window::get_cursor_pos() {
	std::pair<double, double> res;
	glfwGetCursorPos(_handle, &res.first, &res.second);
	return res;
}

std::pair<int, int> window::get_size() {
	std::pair<int, int> res;
	glfwGetFramebufferSize(_handle, &res.first, &res.second);
	return res;
}

void window::run() {
	std::function<void()> update = [this]() {
		glfwPollEvents();
		on_update(_input);
		glfwSwapBuffers(_handle);

		for (size_t i = 0; i < 256; i++) {
			_input.key_press[i] = false;
			_input.key_release[i] = false;
			_input.btn_press[i] = false;
			_input.btn_release[i] = false;
		}
	};

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg([](void *ptr) {
		std::function<void()> *fn = (std::function<void()>*)ptr;
		(*fn)();
	}, &update, 0, true);
#else
	while (!glfwWindowShouldClose(_handle)) {
		update(); 
	}
#endif
}
