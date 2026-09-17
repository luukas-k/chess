#include "board.h"
#include "graphics.h"
#include "window.h"

int main() {
	window wnd;
	
	move_list moves{};
	chess_board board{};
	board.init();
	// init_fen(board, "2n1RR2/p1p1PQp1/3N1r1k/rbBP3P/1Pp1K3/pp1Pb2P/P1p1Pq1p/1N1n4 w - - 0 1");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Input current{};
	// Input prev{};

	Rect rr = create_rect();
	Shader s = create_shader();
	Image pt = create_image();

	wnd.on_update = [&](const Input &in) {
		auto [sw, sh] = wnd.get_size();

		glViewport(0, 0, sw, sh);
		glClearColor(244.f / 255.f, 163.f / 255.f, 132.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		process_input(board, moves, in, sw, sh);
		draw_game(rr, s, pt, board, moves, sw, sh);
	};

	wnd.run();
}