#pragma once

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#else
#include <glad/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <functional>

struct Input {
	bool key_state[256];
	bool key_press[256];
	bool key_release[256];

	bool btn_state[256];
	bool btn_press[256];
	bool btn_release[256];
	int x, y;
};

class window {
public:
	window();
	~window();

	std::function<void(const Input&)> on_update;

	int get_key(int vk);
	int get_button(int vk);
	std::pair<double, double> get_cursor_pos();
	std::pair<int, int> get_size();

	void run();
private:
	GLFWwindow *_handle;
	Input _input;
};