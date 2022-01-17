#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stbi.h"

struct ChessBoard {
	enum PieceType {
		None = 0,
		King,
		Queen,
		Bishop,
		Knight,
		Rook,
		Pawn,
	};
	enum Color {
		Black = 0,
		White = 1 << 4
	};
	constexpr static uint8_t PIECE_BITS = 0b111;
	constexpr static uint8_t COLOR_BIT = 1 << 4;
	uint8_t pieces[8 * 8];
	int8_t selected;
	Color current_turn = Color::White;
};

enum {
	Up = 8,
	Down = -8,
	Left = -1,
	Right = 1
};

void init(ChessBoard& brd) {
	brd.selected = -1;
	for (int i = 0; i < 8 * 8; i++) {
		brd.pieces[i] = ChessBoard::None;
	}
	for (int i = 0; i < 8; i++) {
		brd.pieces[i + 6 * 8] = ChessBoard::Pawn | ChessBoard::Black;
		brd.pieces[i + 1 * 8] = ChessBoard::Pawn | ChessBoard::White;
	}
	brd.pieces[0 + 0 * 8] = ChessBoard::Rook | ChessBoard::Color::White;
	brd.pieces[7 + 0 * 8] = ChessBoard::Rook | ChessBoard::Color::White;
	brd.pieces[1 + 0 * 8] = ChessBoard::Knight | ChessBoard::Color::White;
	brd.pieces[6 + 0 * 8] = ChessBoard::Knight | ChessBoard::Color::White;
	brd.pieces[2 + 0 * 8] = ChessBoard::Bishop | ChessBoard::Color::White;
	brd.pieces[5 + 0 * 8] = ChessBoard::Bishop | ChessBoard::Color::White;
	brd.pieces[3 + 0 * 8] = ChessBoard::Queen | ChessBoard::Color::White;
	brd.pieces[4 + 0 * 8] = ChessBoard::King | ChessBoard::Color::White;

	brd.pieces[0 + 7 * 8] = ChessBoard::Rook | ChessBoard::Color::Black;
	brd.pieces[7 + 7 * 8] = ChessBoard::Rook | ChessBoard::Color::Black;
	brd.pieces[1 + 7 * 8] = ChessBoard::Knight | ChessBoard::Color::Black;
	brd.pieces[6 + 7 * 8] = ChessBoard::Knight | ChessBoard::Color::Black;
	brd.pieces[2 + 7 * 8] = ChessBoard::Bishop | ChessBoard::Color::Black;
	brd.pieces[5 + 7 * 8] = ChessBoard::Bishop | ChessBoard::Color::Black;
	brd.pieces[3 + 7 * 8] = ChessBoard::Queen | ChessBoard::Color::Black;
	brd.pieces[4 + 7 * 8] = ChessBoard::King | ChessBoard::Color::Black;

}

void init_fen(ChessBoard& brd, const char* fen) {
	brd.selected = -1;
	for (int i = 0; i < 8 * 8; i++) {
		brd.pieces[i] = ChessBoard::None;
	}
	int len = strlen(fen);
	int i = 0;
	int cursor = 0;
	for (; i < len; i++) {
		char c = fen[i];
		// 'PNBRQK'
		if (c == 'P') {
			brd.pieces[cursor++] = ChessBoard::Pawn | ChessBoard::White;
		}
		else if (c == 'N') {
			brd.pieces[cursor++] = ChessBoard::Knight | ChessBoard::White;
		}
		else if (c == 'B') {
			brd.pieces[cursor++] = ChessBoard::Bishop | ChessBoard::White;
		}
		else if (c == 'R') {
			brd.pieces[cursor++] = ChessBoard::Rook | ChessBoard::White;
		}
		else if (c == 'Q') {
			brd.pieces[cursor++] = ChessBoard::Queen | ChessBoard::White;
		}
		else if (c == 'K') {
			brd.pieces[cursor++] = ChessBoard::King | ChessBoard::White;
		}
		else if (c == 'p') {
			brd.pieces[cursor++] = ChessBoard::Pawn | ChessBoard::Black;
		}
		else if (c == 'n') {
			brd.pieces[cursor++] = ChessBoard::Knight | ChessBoard::Black;
		}
		else if (c == 'b') {
			brd.pieces[cursor++] = ChessBoard::Bishop | ChessBoard::Black;
		}
		else if (c == 'r') {
			brd.pieces[cursor++] = ChessBoard::Rook | ChessBoard::Black;
		}
		else if (c == 'q') {
			brd.pieces[cursor++] = ChessBoard::Queen | ChessBoard::Black;
		}
		else if (c == 'k') {
			brd.pieces[cursor++] = ChessBoard::King | ChessBoard::Black;
		}
		else if (c == '/') {
			cursor = (cursor - cursor % 8);
		}
		else if (c == ' ') {
			break;
		}
		else {
			cursor += (c - '0');
		}
		if (cursor >= 64) {
			break;
		}
	}
}

struct Rect {
	uint32_t vao, vbo;
};

Rect create_rect() {
	Rect r{};
	glGenVertexArrays(1, &r.vao);
	glBindVertexArray(r.vao);

	glGenBuffers(1, &r.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
	float data[]{
		0.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,

		1.f, 1.f,
		0.f, 0.f,
		1.f, 0.f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, 0);
	glEnableVertexAttribArray(0);

	return r;
}

struct Shader {
	uint32_t prog, vs, fs;
};

Shader create_shader() {
	const char* vs_src = R"GLSL(#version 330 core

layout(location = 0) in vec2 vPos;

uniform vec2 pos = vec2(0);
uniform vec2 scale = vec2(0);

out vec2 fUV;

void main(){
	fUV = vPos;
	gl_Position = vec4((vPos * scale + pos) * 2 - vec2(1), 0, 1);
}

)GLSL";
	const char* fs_src = R"GLSL(#version 330 core

in vec2 fUV;
out vec4 fColor;

uniform vec2 tex_pos = vec2(0); 
uniform vec2 tex_scale = vec2(1); 
uniform sampler2D tex;
uniform vec3 color = vec3(1); 
uniform float color_fac = 1.0f;

void main(){
	vec2 uv = fUV * tex_scale + tex_pos;
	fColor = texture(tex, vec2(uv.x, 1.0 - uv.y)) * (1.0f - color_fac) + color_fac * vec4(color, 1.0);
}

)GLSL";

	Shader s{};
	s.prog = glCreateProgram();

	s.vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(s.vs, 1, &vs_src, nullptr);
	glCompileShader(s.vs);
	glAttachShader(s.prog, s.vs);

	s.fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s.fs, 1, &fs_src, nullptr);
	glCompileShader(s.fs);
	glAttachShader(s.prog, s.fs);

	glLinkProgram(s.prog);

	return s;
}

struct Image {
	uint32_t tex;
};

Image create_image() {
	int w{}, h{}, c{};
	auto data = stbi_load("bin/pieces.png", &w, &h, &c, 4);

	Image img{};
	glGenTextures(1, &img.tex);
	glBindTexture(GL_TEXTURE_2D, img.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	stbi_image_free(data);

	return img;
}

void draw_rect(int x, int y, int w, int h, int sw, int sh, glm::vec3 col) {
	static Rect rr = create_rect();
	static Shader s = create_shader();
	static Image pt = create_image();
	glUseProgram(s.prog);

	glUniform2f(glGetUniformLocation(s.prog, "pos"), (float)x / (float)sw, (float)y / (float)sh);
	glUniform2f(glGetUniformLocation(s.prog, "scale"), (float)w / (float)sw, (float)h / (float)sh);

	glUniform3f(glGetUniformLocation(s.prog, "color"), col.r, col.g, col.b);
	glUniform1f(glGetUniformLocation(s.prog, "color_fac"), 1.f);

	glBindVertexArray(rr.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void draw_piece(int x, int y, int w, int h, int sw, int sh, uint8_t piece) {
	static Rect rr = create_rect();
	static Shader s = create_shader();
	static Image pt = create_image();
	glUseProgram(s.prog);

	glUniform2f(glGetUniformLocation(s.prog, "pos"), (float)x / (float)sw, (float)y / (float)sh);
	glUniform2f(glGetUniformLocation(s.prog, "scale"), (float)w / (float)sw, (float)h / (float)sh);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pt.tex);
	glUniform1i(glGetUniformLocation(s.prog, "tex"), 0);
	float xoff = ((piece & ChessBoard::PIECE_BITS) - 1) * (1.f / 6.f);
	float yoff = (((piece & ChessBoard::COLOR_BIT)) == ChessBoard::Color::White) * 1.f / 2.f;
	glUniform2f(glGetUniformLocation(s.prog, "tex_pos"), xoff, yoff);
	glUniform2f(glGetUniformLocation(s.prog, "tex_scale"), 1.f / 6.f, 1.f / 2.f);
	glUniform1f(glGetUniformLocation(s.prog, "color_fac"), 0.f);

	glBindVertexArray(rr.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLFWwindow* g_window = nullptr;

struct Input {
	bool keys[256];
	bool btns[256];
};

bool key_was_released(const Input& current, const Input& prev, int key) {
	return current.keys[key] == false && prev.keys[key] == true;
}

bool button_was_released(const Input& current, const Input& prev, int btn) {
	return current.btns[btn] == false && prev.btns[btn] == true;
}

void draw_board(const ChessBoard& brd, int offx, int offy, int w, int h, int sw, int sh, const int* highlights, int highlight_count) {
	auto is_light_square = [](int x, int y) {
		return (x % 2 == 0 && y % 2 == 0) || (x % 2 == 1 && y % 2 == 1);
	};
	auto get_color = [&](int x, int y) -> glm::vec3 {
		for (int i = 0; i < 64; i++) {
			if (highlights[i] == -1)
				break;
			if (highlights[i] == x + y * 8) {
				return { 0.6f, 0.4f, 0.4f };
			}
		}
		if (x == (brd.selected % 8) && y == (brd.selected / 8)) {
			return { 0.6f, 0.4f, 0.4f };
		}
		if (is_light_square(x, y)) // light
			return { 125.f / 255.f, 135.f / 255.f, 150.f / 255.f };
		else // dark
			return { 232.f / 255.f, 235.f / 255.f, 239.f / 255.f };
	};
	for (int i = 0; i < 8 * 8; i++) {
		int px = (i % 8) * w + offx;
		int py = (i / 8) * h + offy;
		auto col = get_color((i % 8), (i / 8));
		draw_rect(px, py, w, h, sw, sh, col);
		if ((brd.pieces[i] & ChessBoard::PIECE_BITS) != 0)
			draw_piece(px, py, w, h, sw, sh, brd.pieces[i]);
	}
}

bool in_range(int val, int min_inc, int max_ex) {
	return (val >= min_inc) && (val < max_ex);
}

void draw(ChessBoard& brd, const Input& cur_in, const Input& prev_in, int cx, int cy, int sw, int sh) {
	int h = 0;
	int w = 0;
	int offx = 0;
	int offy = 0;
	if (sw >= sh) {
		h = sh / 8;
		w = h;
		offx = (sw - w * 8) / 2;
	}
	else {
		w = sw / 8;
		h = w;
		offy = (sh - h * 8) / 2;
	}

	int hx = 0;
	int hy = 0;
	bool on_screen = false;
	if (cx > offx && cx < sw - offx && cy > offy && cy < sh - offy) {
		hx = ((cx - offx) / w);
		hy = ((cy - offy) / h);
		on_screen = true;
	}

	auto is_enemy = [&](int self, int p) {
		return ((brd.pieces[self] & ChessBoard::COLOR_BIT) != (brd.pieces[p] & ChessBoard::COLOR_BIT));
	};
	auto is_empty = [&](int p) {
		return ((brd.pieces[p] & ChessBoard::PIECE_BITS) == 0);
	};
	auto is_enemy_or_empty = [&](int self, int p) {
		return is_enemy(self, p) || is_empty(p);
	};

	if (button_was_released(cur_in, prev_in, GLFW_MOUSE_BUTTON_1)) {
		if (brd.selected == -1){
			brd.selected = hx + hy * 8;
			if (is_empty(brd.selected) || (brd.pieces[brd.selected] & ChessBoard::COLOR_BIT) != brd.current_turn) {
				brd.selected = -1;
			}
		}
	}

	int highlights[64]{};
	int move_count = 0;
	for (int i = 0; i < 64; i++)
		highlights[i] = -1;

	auto is_own = [&](int self, int p) {
		return (brd.pieces[self] & ChessBoard::COLOR_BIT) == (brd.pieces[p] & ChessBoard::COLOR_BIT) && ((brd.pieces[p] & ChessBoard::PIECE_BITS) != 0);
	};
	auto get_pawn_moves = [&](int* h, int p) {
		int x = p % 8;
		int y = p / 8;
		int n = 0;
		if ((brd.pieces[p] & ChessBoard::COLOR_BIT) == ChessBoard::Color::Black) {
			if(is_empty(p + Up)){
				h[n++] = p + Up;
			}
			if(is_empty(p + Up) && is_empty(p + Up * 2) && y == 1){
				h[n++] = p + Up * 2;
			}
			if (is_enemy(p, p + Up + Left) && !is_empty(p + Up + Left) && x > 0) {
				h[n++] = p + Up + Left;
			}
			if (is_enemy(p, p + Up + Right) && !is_empty(p + Up + Right) && x < 7) {
				h[n++] = p + Up + Right;
			}
		}
		else {
			if(is_empty(p + Down)){
				h[n++] = p + Down;
			}
			if(is_empty(p + Down) && is_empty(p + Down * 2) && y == 6){
				h[n++] = p + Down * 2;
			}
			if (is_enemy(p, p + Down + Left) && !is_empty(p + Down + Left) && x > 0) {
				h[n++] = p + Down + Left;
			}
			if (is_enemy(p, p + Down + Right) && !is_empty(p + Down + Right) && x < 7) {
				h[n++] = p + Down + Right;
			}
		}
		return n;
	};
	auto get_knight_moves = [&](int* h, int p) {
		int x = p % 8;
		int y = p / 8;
		int n = 0;
		if (x > 0 && y < 6 && is_enemy_or_empty(p, p + 2 * Up + Left))
			h[n++] = p + 2 * Up + Left;
		if (x < 7 && y < 6 && is_enemy_or_empty(p, p + 2 * Up + Right))
			h[n++] = p + 2 * Up + Right;
		if (x > 0 && y > 1 && is_enemy_or_empty(p, p + 2 * Down + Left))
			h[n++] = p + 2 * Down + Left;
		if (x < 7 && y > 1 && is_enemy_or_empty(p, p + 2 * Down + Right))
			h[n++] = p + 2 * Down + Right;
		if (x > 2 && y < 7 && is_enemy_or_empty(p, p + 2 * Left + Up))
			h[n++] = p + 2 * Left + Up;
		if (x > 2 && y > 0 && is_enemy_or_empty(p, p + 2 * Left + Down))
			h[n++] = p + 2 * Left + Down;
		if (x < 6 && y < 7 && is_enemy_or_empty(p, p + 2 * Right + Up))
			h[n++] = p + 2 * Right + Up;
		if (x < 6 && y > 0 && is_enemy_or_empty(p, p + 2 * Right + Down))
			h[n++] = p + 2 * Right + Down;
		return n;
	};
	auto get_bishop_moves = [&](int* h, int p) {
		int x = p % 8;
		int y = p / 8;
		int n = 0;
		int directions[]{ Up + Left, Up + Right, Down + Left, Down + Right };
		int dir_count = 4;
		auto add = [&](int xx, int yy) {
			if (xx < 8 && xx >= 0 && yy >= 0 && yy < 8)
				h[n++] = xx + yy * 8;
		};
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Up + Right) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Up + Left) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Down + Right) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Down + Left) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		return n;
	};
	auto get_queen_moves = [&](int* h, int p) {
		int x = p % 8;
		int y = p / 8;
		int n = 0;
		auto add = [&](int xx, int yy) {
			if (xx < 8 && xx >= 0 && yy >= 0 && yy < 8)
				h[n++] = xx + yy * 8;
		};
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Up + Right) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Up + Left) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Down + Right) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Down + Left) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Up) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Right) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < (8 - x - 1); i++) {
			int np = p + (Down) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		for (int i = 0; i < x; i++) {
			int np = p + (Left) * (i + 1);
			int nx = np % 8;
			int ny = np / 8;
			if (is_own(p, np)) {
				break;
			}
			add(nx, ny);
			if (is_enemy(p, np)) {
				break;
			}
		}
		return n;
	};
	auto get_moves = [&](int* h, int p) {
		int n = 0;
		if ((brd.pieces[p] & ChessBoard::PIECE_BITS) == ChessBoard::Pawn)
			n += get_pawn_moves(h + n, p);
		if ((brd.pieces[p] & ChessBoard::PIECE_BITS) == ChessBoard::Knight)
			n += get_knight_moves(h + n, p);
		if ((brd.pieces[p] & ChessBoard::PIECE_BITS) == ChessBoard::Bishop)
			n += get_bishop_moves(h + n, p);
		if ((brd.pieces[p] & ChessBoard::PIECE_BITS) == ChessBoard::Queen)
			n += get_queen_moves(h + n, p);
		return n;
	};
	if (brd.selected != -1) {
		move_count += get_moves(highlights, brd.selected);
		if (move_count == 0) {
			brd.selected = -1;
		}
	}

	if (button_was_released(cur_in, prev_in, GLFW_MOUSE_BUTTON_1)) {
		int off = hx + hy * 8;
		for (int k = 0; k < 64; k++) {
			if (highlights[k] != -1 && highlights[k] == off) {
				if(in_range(brd.selected, 0, 64)){
					brd.pieces[off] = brd.pieces[brd.selected];
					brd.pieces[brd.selected] = 0;
					brd.selected = -1;
					if(brd.current_turn == ChessBoard::Color::Black)
						brd.current_turn = ChessBoard::Color::White;
					else
						brd.current_turn = ChessBoard::Color::Black;
				}
			}
		}
	}

	if(on_screen){
		highlights[move_count++] = hx + hy * 8;
	}

	draw_board(brd, offx, offy, w, h, sw, sh, highlights, move_count);
}

#include <iostream>



int main() {
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "title", nullptr, nullptr);
	g_window = window;
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		std::cout << message << std::endl;
	}, nullptr);

	ChessBoard board{};
	// init(board);
	init_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	// init_fen(board, "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Input current{};
	Input prev{};

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		prev = current;
		for (int i = 0; i < 256; i++) {
			current.keys[i] = glfwGetKey(window, i);
			current.btns[i] = glfwGetMouseButton(window, i);
		}

		int sw, sh;
		glfwGetFramebufferSize(window, &sw, &sh);

		glViewport(0, 0, sw, sh);

		glClearColor(244.f / 255.f, 163.f / 255.f, 132.f / 255.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		double cx{}, cy{};
		glfwGetCursorPos(window, &cx, &cy);

		draw(board, current, prev, cx, sh - (int)cy, sw, sh);

		glfwSwapBuffers(window);
	}
	// OS cleanup
}