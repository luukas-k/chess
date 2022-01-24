#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

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
	// Board state
	uint8_t pieces[8 * 8]{};
	Color current_turn = Color::White;
	// Currently selected piece on the board or in the pawn promotion menu
	int8_t selected{ -1 };
	// Pawn capture information
	int en_passant_target{ -1 };
	// Available moves
	int move_list[65]{};
	int move_count = 0;
	// King information
	bool is_check{ false }, is_checkmate{ false };
	int white_king_position{ 0 }, black_king_position{ 0 };
	// Square hovered by cursor
	int hovered_square{ -1 };
	// Pawn promotion info
	int to_be_promoted{ -1 };
	bool wait_for_promotion_selection{ false };
	// Castling availability
	bool
		black_king_side{ true },
		black_queen_side{ true },
		white_king_side{ true },
		white_queen_side{ true };
};

// Directions set to offsets in an array that correspond to movements on the grid
enum {
	Up = 8,
	Down = -8,
	Left = -1,
	Right = 1
};

enum struct IgnoreInvalid { _ };

struct Position {
	Position(int p) : p(p) { assert(p >= 0); assert(p < 64); }
	Position(int p, IgnoreInvalid i) : p(p) {}
	Position offset(int mv) { return Position(p + mv); }
	int p{};
	int x() const { return p % 8; }
	int y() const { return p / 8; }
	bool is_valid() { return (x() >= 0) && (x() < 8) && (y() >= 0) && (y() < 8); }
};

ChessBoard::Color get_color(const ChessBoard& brd, Position p) {
	ChessBoard::Color color = (ChessBoard::Color)(brd.pieces[p.p] & ChessBoard::COLOR_BIT);
	return color;
}
ChessBoard::PieceType get_type(const ChessBoard& brd, Position p) {
	ChessBoard::PieceType type = (ChessBoard::PieceType)(brd.pieces[p.p] & ChessBoard::PIECE_BITS);
	return type;
}
int get_piece(const ChessBoard& brd, Position p) {
	return brd.pieces[p.p];
}

void init_fen(ChessBoard& brd, const char* fen) {
	brd.selected = -1;
	for (int i = 0; i < 8 * 8; i++) {
		brd.pieces[i] = ChessBoard::None;
	}
	int len = strlen(fen);
	int i = 0;
	int cursor = 0;
	// Board state
	for (; i < len; i++) {
		char c = fen[i];
		// 'PNBRQK'
		if (i <= 64) {
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
				brd.white_king_position = cursor;
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
				brd.black_king_position = cursor;
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
	// Turn
	char turn = *(fen + i + 2);
	if (turn == 'w') {
		brd.current_turn = ChessBoard::White;
	}
	else if (turn == 'b') {
		brd.current_turn = ChessBoard::Black;
	}
	else {
		assert(false);
	}
	// Castling availability
	const char* castling = fen + i + 2 + 2;
	//if (castling[0] == '-') {
	//	// No castling available
	//}
	//else {

	//}
	return;
}

void init(ChessBoard& brd) {
	init_fen(brd, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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

struct Input {
	bool keys[256];
	bool btns[256];
	int x, y;
};

bool key_was_released(const Input& current, const Input& prev, int key) {
	return current.keys[key] == false && prev.keys[key] == true;
}

bool button_was_released(const Input& current, const Input& prev, int btn) {
	return current.btns[btn] == false && prev.btns[btn] == true;
}

void draw_board(const ChessBoard& brd, int offx, int offy, int w, int h, int sw, int sh) {
	auto is_light_square = [](int x, int y) {
		return (x % 2 == 0 && y % 2 == 0) || (x % 2 == 1 && y % 2 == 1);
	};
	auto is_checked_king = [&](int x, int y) {
		if (brd.is_check) {
			return brd.current_turn == ChessBoard::White ?
				(brd.white_king_position == x + y * 8) :
				(brd.black_king_position == x + y * 8);
		}
		return false;
	};
	auto is_selected = [&](int x, int y) {
		return x == (brd.selected % 8) && y == (brd.selected / 8);
	};
	auto is_move = [&](int x, int y) {
		for (int i = 0; i < brd.move_count; i++) {
			if (brd.move_list[i] == (x + y * 8)) {
				return true;
			}
		}
		return false;
	};
	auto is_hovered = [&](int x, int y) {
		return brd.hovered_square == (x + y * 8);
	};
	auto get_color = [&](int x, int y) -> glm::vec3 {
		// Checked king
		if (is_checked_king(x, y)) {
			return { 0.9f, 0.1f, 0.1f };
		}
		// Mouse hover
		if (is_hovered(x, y) && !brd.wait_for_promotion_selection) {
			return { 0.7f, 0.3f, 0.3f };
		}
		// Possible moves
		if (is_move(x, y)) {
			return { 0.5f, 0.3f, 0.3f };
		}
		// Selected piece
		if (is_selected(x, y) && !brd.wait_for_promotion_selection) {
			return { 0.9f, 0.4f, 0.4f };
		}
		// Background
		if (is_light_square(x, y)) // light
			return { 0.5f, 0.52f, 0.6f };
		else // dark
			return { 0.9f, 0.92f, 0.93f };
	};
	for (int i = 0; i < 8 * 8; i++) {
		int px = (i % 8) * w + offx;
		int py = (i / 8) * h + offy;
		auto col = get_color((i % 8), (i / 8));
		draw_rect(px, py, w, h, sw, sh, col);
		if (get_type(brd, i) != 0)
			draw_piece(px, py, w, h, sw, sh, get_piece(brd, i));
	}

	if (brd.wait_for_promotion_selection) {
		// Promotion select bg
		draw_rect(2 * w + offx, 3.5 * h + offy, w * 4, h, sw, sh, { 0.2f, 0.2f, 0.2f });
		// Promotion select highlight
		if (brd.selected != -1) {
			int selection_highlight = brd.selected;
			draw_rect((2 + selection_highlight) * w + offx, 3.5 * h + offy, w, h, sw, sh, { 0.4f, 0.2f, 0.2f });
		}
		int protion_pieces[]{ ChessBoard::Queen, ChessBoard::Rook, ChessBoard::Bishop, ChessBoard::Knight };
		for (int a = 0; a < 4; a++) {
			int px = (2 + a) * w + offx;
			int py = 3.5 * h + offy;
			draw_piece(px, py, w, h, sw, sh, protion_pieces[a]);
		}
	}
}

bool in_range(int val, int min_inc, int max_ex) {
	return (val >= min_inc) && (val < max_ex);
}

bool is_empty(const ChessBoard& brd, Position p) {
	return get_type(brd, p) == ChessBoard::None;
};

bool is_enemy(const ChessBoard& brd, Position self, Position p) {
	return get_color(brd, self) != get_color(brd, p) && !is_empty(brd, p);
};


bool is_enemy_or_empty(const ChessBoard& brd, int self, int p) {
	return is_enemy(brd, self, p) || is_empty(brd, p);
};

bool is_own(const ChessBoard& brd, int self, int p) {
	return
		get_color(brd, self) == get_color(brd, p) &&
		get_type(brd, self) != ChessBoard::None &&
		get_type(brd, p) != ChessBoard::None;
};

bool is_in_check(const ChessBoard& brd, ChessBoard::Color c);

void do_move(ChessBoard& brd, Position from_pos, Position to_pos);

bool resolves_check(const ChessBoard& brd, Position pos, Position target) {
	ChessBoard copy = brd;
	do_move(copy, pos, target);
	return !is_in_check(copy, brd.current_turn);
};

bool causes_check_on_self(const ChessBoard& brd, Position pos, Position target) {
	ChessBoard copy = brd;
	do_move(copy, pos, target);
	return is_in_check(copy, brd.current_turn);
};

void add_move(const ChessBoard& brd, int* move_list, int& move_count, Position pos, int move) {
	Position targ = pos.offset(move);
	if (!is_empty(brd, targ) && (get_color(brd, pos) == get_color(brd, targ))) {
		return;
	}
	assert(move_count < 64);
	move_list[move_count] = targ.p;
	move_count += 1;
};

bool is_valid(Position p, int move) {
	auto np = Position(p.p + move, IgnoreInvalid::_);
	return np.is_valid();
};

void get_pawn_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	int dir =
		((brd.pieces[(int)p.p] & ChessBoard::COLOR_BIT) == ChessBoard::Color::Black) ?
		Up : Down;
	bool can_move = (dir == Up) ? (p.y() != 7) : (p.y() != 0);
	bool can_double_move = (dir == Up) ? (p.y() == 1) : (p.y() == 6);

	if (!can_move) {
		return;
	}
	if (is_empty(brd, p.offset(dir))) {
		add_move(brd, move_list, move_count, p, dir);
		if (can_double_move && is_empty(brd, p.offset(dir * 2))) {
			add_move(brd, move_list, move_count, p, dir * 2);
		}
	}
	auto can_enpassant = [&](Position from, bool towards_left) {
		if (brd.en_passant_target == -1) return false;

		Position enemy_pawn = from.offset(towards_left ? Left : Right);
		if (towards_left) {
			if (enemy_pawn.x() == from.x() - 1) {
				if (brd.en_passant_target == enemy_pawn.p) {
					return true;
				}
			}
		}
		else {
			if (enemy_pawn.x() == from.x() + 1) {
				if (brd.en_passant_target == enemy_pawn.p) {
					return true;
				}
			}
		}
		return false;
	};
	if (p.x() != 0) {
		if (!is_empty(brd, p.offset(dir + Left)) && is_enemy(brd, p, p.offset(dir + Left))) {
			add_move(brd, move_list, move_count, p, dir + Left);
		}
		if (can_enpassant(p, true)) {
			add_move(brd, move_list, move_count, p, dir + Left);
		}
	}
	if (p.x() != 7) {
		if (!is_empty(brd, p.offset(dir + Right)) && is_enemy(brd, p, p.offset(dir + Right))) {
			add_move(brd, move_list, move_count, p, dir + Right);
		}
		if (can_enpassant(p, false)) {
			add_move(brd, move_list, move_count, p, dir + Right);
		}
	}
};
void get_knight_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	// return;

	int possible_moves[]{ Left * 2 + Up, Left + 2 * Up, Right + 2 * Up, Right * 2 + Up, Right * 2 + Down, Right + Down * 2, Left + Down * 2, Left * 2 + Down };
	int move_delta_x[]{ -2, -1,1, 2,2,1,-1,-2 };
	int move_delta_y[]{ 1,2,2,1,-1,-2,-2, -1 };
	int possible_move_cnt = 8;
	for (int i = 0; i < possible_move_cnt; i++) {
		int dx = move_delta_x[i];
		int dy = move_delta_y[i];
		int nx = p.x() + dx;
		int ny = p.y() + dy;

		// /	Position target(p.p + possible_moves[i], IgnoreInvalid::_);
		if (!(nx >= 0 && nx < 8 && ny >= 0 && ny < 8))
			continue;
		// Check that move is valid

		if (is_valid(p, possible_moves[i])) {
			if (is_empty(brd, p.offset(possible_moves[i]).p) || (!is_empty(brd, p.offset(possible_moves[i]).p) && is_enemy(brd, p, p.offset(possible_moves[i]))))
				add_move(brd, move_list, move_count, p, possible_moves[i]);
		}
	}
};
int min(int a, int b) {
	return a < b ? a : b;
};
void get_bishop_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	int possible_dirs[]{ Left + Up, Right + Up, Right + Down, Left + Down };
	int possible_dir_count = 4;
	int max_moves_left = p.x();
	int max_moves_right = 8 - p.x() - 1;
	int max_moves_down = p.y();
	int max_moves_up = 8 - p.y() - 1;
	int max_moves_dir[]{
		min(max_moves_left, max_moves_up),
		min(max_moves_right, max_moves_up),
		min(max_moves_right, max_moves_down),
		min(max_moves_left, max_moves_down),
	};
	for (int i = 0; i < possible_dir_count; i++) {
		int max_moves_direction = max_moves_dir[i];
		if (max_moves_direction > 0) {
			Position tp = p;
			for (int dist = 0; dist < max_moves_direction; dist++) {
				tp = tp.offset(possible_dirs[i]);
				if (!is_empty(brd, tp.p) && is_enemy(brd, p, tp)) {
					add_move(brd, move_list, move_count, p, tp.p - p.p);
					break;
				}
				if (is_own(brd, p.p, tp.p)) {
					break;
				}
				add_move(brd, move_list, move_count, p, tp.p - p.p);
			}
		}
	}
};
void get_queen_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	int possible_dirs[]{ Left + Up, Right + Up, Right + Down, Left + Down, Left, Up, Right, Down };
	int possible_dir_count = 8;
	int max_moves_left = p.x();
	int max_moves_right = 8 - p.x() - 1;
	int max_moves_down = p.y();
	int max_moves_up = 8 - p.y() - 1;
	int max_moves_dir[]{
		min(max_moves_left, max_moves_up),
		min(max_moves_right, max_moves_up),
		min(max_moves_right, max_moves_down),
		min(max_moves_left, max_moves_down),
		max_moves_left,
		max_moves_up,
		max_moves_right,
		max_moves_down
	};
	for (int i = 0; i < possible_dir_count; i++) {
		int max_moves_direction = max_moves_dir[i];
		if (max_moves_direction > 0) {
			Position tp = p;
			for (int dist = 0; dist < max_moves_direction; dist++) {
				// tp = tp.offset(possible_dirs[i]);
				if (!is_empty(brd, p.p + possible_dirs[i] * (dist + 1)) && is_enemy(brd, p, p.p + possible_dirs[i] * (dist + 1))) {
					add_move(brd, move_list, move_count, p, possible_dirs[i] * (dist + 1));
					break;
				}
				if (is_own(brd, p.p, p.p + possible_dirs[i] * (dist + 1))) {
					break;
				}
				add_move(brd, move_list, move_count, p, possible_dirs[i] * (dist + 1));
			}
		}
	}
};
void get_rook_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	int possible_dirs[]{ Left, Up, Right, Down };
	int possible_dir_count = 4;
	int max_moves_left = p.x();
	int max_moves_right = 8 - p.x() - 1;
	int max_moves_down = p.y();
	int max_moves_up = 8 - p.y() - 1;
	int max_moves_dir[]{
		max_moves_left,
		max_moves_up,
		max_moves_right,
		max_moves_down
	};
	for (int i = 0; i < possible_dir_count; i++) {
		int max_moves_direction = max_moves_dir[i];
		if (max_moves_direction > 0) {
			Position tp = p;
			for (int dist = 0; dist < max_moves_direction; dist++) {
				// tp = tp.offset(possible_dirs[i]);
				if (!is_empty(brd, p.p + possible_dirs[i] * (dist + 1)) && is_enemy(brd, p, p.p + possible_dirs[i] * (dist + 1))) {
					add_move(brd, move_list, move_count, p, possible_dirs[i] * (dist + 1));
					break;
				}
				if (is_own(brd, p.p, p.p + possible_dirs[i] * (dist + 1))) {
					break;
				}
				add_move(brd, move_list, move_count, p, possible_dirs[i] * (dist + 1));
			}
		}
	}
};
void get_king_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	int possible_dirs[]{ Left + Up, Right + Up, Right + Down, Left + Down, Up, Down, Left, Right };
	int possible_dir_count = 8;
	for (int i = 0; i < possible_dir_count; i++) {
		if (is_valid(p, possible_dirs[i])) {
			if (is_empty(brd, p.offset(possible_dirs[i]).p) || (!is_empty(brd, p.offset(possible_dirs[i]).p) && is_enemy(brd, p, p.offset(possible_dirs[i]))))
				add_move(brd, move_list, move_count, p, possible_dirs[i]);
		}
	}

	// bool can_king_side_castle = 
};

void get_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	auto type = get_type(brd, p);
	if (type == ChessBoard::Pawn)
		get_pawn_moves(brd, move_list, move_count, p);
	else if (type == ChessBoard::Knight)
		get_knight_moves(brd, move_list, move_count, p);
	else if (type == ChessBoard::Bishop)
		get_bishop_moves(brd, move_list, move_count, p);
	else if (type == ChessBoard::Queen)
		get_queen_moves(brd, move_list, move_count, p);
	else if (type == ChessBoard::King)
		get_king_moves(brd, move_list, move_count, p);
	else if (type == ChessBoard::Rook)
		get_rook_moves(brd, move_list, move_count, p);
	else {
		move_count = 0;
	}
};

void get_valid_moves(const ChessBoard& brd, int* move_list, int& move_count, Position p) {
	get_moves(brd, move_list, move_count, p);
	int valid_move_count = 0;
	for (int mv_i = 0; mv_i < move_count; mv_i++) {
		if (brd.is_check) {
			if (resolves_check(brd, p, move_list[mv_i])) {
				move_list[valid_move_count] = move_list[mv_i];
				valid_move_count++;
			}
		}
		else {
			if (!causes_check_on_self(brd, p, move_list[mv_i])) {
				move_list[valid_move_count] = move_list[mv_i];
				valid_move_count++;
			}
		}
	}
	move_count = valid_move_count;
}

bool is_in_check(const ChessBoard& brd, ChessBoard::Color c) {
	Position king_location = (c == ChessBoard::White) ? brd.white_king_position : brd.black_king_position;
	ChessBoard::Color opposingColor = (c == ChessBoard::White) ? ChessBoard::Black : ChessBoard::White;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!is_empty(brd, i) && ((get_color(brd, from) == opposingColor))) {
			int piece_move_list[64]{ -1 };
			int piece_move_count = 0;
			get_moves(brd, piece_move_list, piece_move_count, from);
			for (int mv_i = 0; mv_i < piece_move_count; mv_i++) {
				Position target(piece_move_list[mv_i]);
				if (target.p == king_location.p) {
					return true;
				}
			}
		}
	}
	return false;
};

bool is_in_checkmate(const ChessBoard& brd, ChessBoard::Color c) {
	Position king_location = (c == ChessBoard::White) ? brd.white_king_position : brd.black_king_position;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!is_empty(brd, i) && ((get_color(brd, from) == c))) {
			int piece_move_list[64]{ -1 };
			int piece_move_count = 0;
			get_valid_moves(brd, piece_move_list, piece_move_count, from);
			if (piece_move_count != 0) {
				return false;
			}
		}
	}
	return true;
};

void do_move(ChessBoard& brd, Position from_pos, Position to_pos) {
	if (from_pos.p == to_pos.p)
		return;
	// for (int k = 0; k < 64; k++) {
		// Check if valid move
		// if (brd.highlights[k] != -1 && brd.highlights[k] == to_pos) {
			// if (in_range(brd.selected, 0, 64)) {
				// Was valid so do the move
	if (get_type(brd, from_pos) == ChessBoard::Pawn) {
		// Was a pawn
		int py = to_pos.y();
		int dy = from_pos.y() > py ? from_pos.y() - py : py - from_pos.y();
		if (dy == 2) {
			// Was double move so update en passant target
			brd.en_passant_target = to_pos.p;
		}
		else {
			// Was single move so check wether it was en passant capture
			int px = from_pos.x();
			int dx = from_pos.x() > px ? from_pos.x() - px : px - from_pos.x();
			if (dx == 1) {
				// Was en passant so clear en passant target and capture the pawn there
				brd.pieces[brd.en_passant_target] = 0;
				brd.en_passant_target = -1;
			}
			// Wasn't double move so clear en passant target
			brd.en_passant_target = -1;
		}
		if ((to_pos.y() == 7) || (to_pos.y() == 0)) {
			brd.to_be_promoted = to_pos.p;
			brd.wait_for_promotion_selection = true;
		}
	}
	else {
		brd.en_passant_target = -1;
	}
	// Update king positions if king was moved
	if (brd.pieces[from_pos.p] == (ChessBoard::Color::White | ChessBoard::King)) {
		brd.white_king_position = to_pos.p;
	}
	if (brd.pieces[from_pos.p] == (ChessBoard::Color::Black | ChessBoard::King)) {
		brd.black_king_position = to_pos.p;
	}

	brd.pieces[to_pos.p] = brd.pieces[from_pos.p];
	brd.pieces[from_pos.p] = 0;

	if (brd.current_turn == ChessBoard::Color::Black) {
		brd.current_turn = ChessBoard::Color::White;
	}
	else {
		brd.current_turn = ChessBoard::Color::Black;
	}
};

void process_input(ChessBoard& brd, const Input& cin, const Input& pin, int sw, int sh) {
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
	int
		cx = cin.x,
		cy = sh - cin.y;
	if (cx > offx && cx < sw - offx && cy > offy && cy < sh - offy) {
		hx = ((cx - offx) / w);
		hy = ((cy - offy) / h);
		on_screen = true;
	}

	brd.move_count = 0;
	for (int i = 0; i < 64; i++)
		brd.move_list[i] = -1;

	if (!brd.is_checkmate) {
		if (!brd.wait_for_promotion_selection) {
			if (button_was_released(cin, pin, GLFW_MOUSE_BUTTON_1)) {
				if (brd.selected == -1) {
					brd.selected = hx + hy * 8;
					// int selected_piece_color = (brd.pieces[brd.selected] & ChessBoard::COLOR_BIT);
					ChessBoard::Color pieceColor = get_color(brd, brd.selected);
					if (is_empty(brd, brd.selected) || (pieceColor != brd.current_turn)) {
						brd.selected = -1;
					}
				}
				else {
					if (brd.selected == (hx + hy * 8)) {
						brd.selected = -1;
					}
					else if (is_own(brd, brd.selected, hx + hy * 8)) {
						brd.selected = hx + hy * 8;
					}
				}
			}


			if (brd.selected != -1) {
				get_valid_moves(brd, brd.move_list, brd.move_count, brd.selected);
				if (brd.move_count == 0) {
					brd.selected = -1;
				}
			}
			else {
				for (int i = 0; i < brd.move_count; i++) {
					brd.move_list[i] = -1;
				}
			}

			// Do the move
			if (button_was_released(cin, pin, GLFW_MOUSE_BUTTON_1)) {
				// Move target
				int move_target = hx + hy * 8;
				if (in_range(move_target, 0, 64)) {
					for (int i = 0; i < brd.move_count; i++) {
						if ((brd.move_list[i] != -1) && (brd.move_list[i] == move_target)) {
							do_move(brd, brd.selected, move_target);
							brd.is_check = false;
							if (is_in_checkmate(brd, brd.current_turn)) {
								brd.is_checkmate = true;
								std::cout << "Check mate!" << std::endl;
							}
							else if (is_in_check(brd, brd.current_turn)) {
								brd.is_check = true;
								std::cout << "Check!" << std::endl;
							}
							break;
						}
					}
				}
			}

			if (on_screen) {
				brd.hovered_square = hx + hy * 8;
			}
			else {
				brd.hovered_square = -1;
			}
		}
		else {
			int sel_offx = (cin.x - offx) / w - 2;
			int sel_y = (cin.y + h / 2 - offy) / h - 4;
			if (sel_offx >= 0 && sel_offx <= 4 && (sel_y == 0)) {
				brd.selected = sel_offx;
				if (button_was_released(cin, pin, GLFW_MOUSE_BUTTON_1)) {
					int promotion_pieces[]{ ChessBoard::Queen, ChessBoard::Rook, ChessBoard::Bishop, ChessBoard::Knight };
					ChessBoard::Color teamToPromote = brd.current_turn == ChessBoard::White ? ChessBoard::Black : ChessBoard::White;
					brd.pieces[brd.to_be_promoted] = promotion_pieces[brd.selected] | teamToPromote;
					brd.to_be_promoted = -1;
					brd.wait_for_promotion_selection = false;
					brd.selected = -1;
					brd.is_check = false;
					if (is_in_checkmate(brd, brd.current_turn)) {
						brd.is_checkmate = true;
						std::cout << "Check mate!" << std::endl;
					}
					else if (is_in_check(brd, brd.current_turn)) {
						brd.is_check = true;
						std::cout << "Check!" << std::endl;
					}
				}
			}
			else {
				brd.selected = -1;
			}
		}
	}
	else {
		brd.selected = -1;
	}

	if (key_was_released(cin, pin, GLFW_KEY_R)) {
		init(brd);
	}
}

void draw(ChessBoard& brd, int sw, int sh) {
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
	draw_board(brd, offx, offy, w, h, sw, sh);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Chess", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		std::cout << message << std::endl;
	}, nullptr);

	ChessBoard board{};
	init(board);
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
		draw(board, sw, sh);

		glfwSwapBuffers(window);
	}
	// OS will do the cleanup on app exit so don't even bother
}