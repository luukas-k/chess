#include "board.h"
#include "graphics.h"

int main() {
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Chess", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	#ifndef __EMSCRIPTEN__
	gladLoadGL((GLADloadfunc)glfwGetProcAddress);
	
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		std::cout << message << std::endl;
	}, nullptr);
	#endif
	
	ChessBoard board{};
	init(board);
	// init_fen(board, "2n1RR2/p1p1PQp1/3N1r1k/rbBP3P/1Pp1K3/pp1Pb2P/P1p1Pq1p/1N1n4 w - - 0 1");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Input current{};
	Input prev{};

	Rect rr = create_rect();
	Shader s = create_shader();
	Image pt = create_image();

	std::function<void()> update = [&]() {
		glfwPollEvents();

		prev = current;
		for (int i = 0; i < 256; i++) {
			current.keys[i] = glfwGetKey(window, i);
			current.btns[i] = glfwGetMouseButton(window, i);
		}

		double cx{}, cy{};
		glfwGetCursorPos(window, &cx, &cy);

		current.x = (int)cx;
		current.y = (int)cy;

		int sw, sh;
		glfwGetFramebufferSize(window, &sw, &sh);

		glViewport(0, 0, sw, sh);

		glClearColor(244.f / 255.f, 163.f / 255.f, 132.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		process_input(board, current, prev, sw, sh);
		draw(rr, s, pt, board, sw, sh);

		glfwSwapBuffers(window);
	};

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg([](void *ptr) {
		std::function<void()> *fn = (std::function<void()>*)ptr;
		(*fn)();
	}, &update, 0, true);
#else
	while (!glfwWindowShouldClose(window)) {
		update();
	}
#endif


	// OS will do the cleanup on app exit so don't even bother
}