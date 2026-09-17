#include "graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

Shader create_shader() {
#ifndef __EMSCRIPTEN__
	const char *vs_src = R"GLSL(#version 330 core

layout(location = 0) in vec2 vPos;

uniform vec2 pos = vec2(0);
uniform vec2 scale = vec2(0);

out vec2 fUV;

void main(){
	fUV = vPos;
	gl_Position = vec4((vPos * scale + pos) * 2 - vec2(1), 0, 1);
}

)GLSL";
	const char *fs_src = R"GLSL(#version 330 core

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
#else
	const char *vs_src = R"GLSL(#version 300 es
	
	precision highp float;

	layout(location = 0) in vec2 vPos;

	uniform vec2 pos;
	uniform vec2 scale;

	out vec2 fUV;

	void main() {
		gl_Position = vec4((vPos * scale + pos) * 2.0 - vec2(1.0), 0.0, 1.0);
		fUV = vPos;
	}
	
	)GLSL";
	const char *fs_src = R"GLSL(#version 300 es

	precision highp float;
	
	in vec2 fUV;
	out vec4 fColor;

	uniform vec2 tex_pos;
	uniform vec2 tex_scale; 
	uniform sampler2D tex;
	uniform vec3 color; 
	uniform float color_fac;

	void main(){
		vec2 uv = fUV * tex_scale + tex_pos;
		fColor = texture(tex, vec2(uv.x, 1.0 - uv.y)) * (1.0 - color_fac) + color_fac * vec4(color, 1.0);
	}

	)GLSL";
#endif

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

	int len = 0;
	glGetProgramiv(s.prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		std::string log;
		log.resize((size_t)len);
		glGetProgramInfoLog(s.prog, log.size(), &len, (char *)log.data());
	}

	s.loc_pos = glGetUniformLocation(s.prog, "pos");
	s.loc_scale = glGetUniformLocation(s.prog, "scale");
	s.loc_tex_pos = glGetUniformLocation(s.prog, "tex_pos");
	s.loc_tex_scale = glGetUniformLocation(s.prog, "tex_scale");
	s.loc_tex = glGetUniformLocation(s.prog, "tex");
	s.loc_color = glGetUniformLocation(s.prog, "color");
	s.loc_color_fac = glGetUniformLocation(s.prog, "color_fac");

	return s;
}

Image create_image() {
	int w{}, h{}, c{};
	auto data = stbi_load("assets/pieces.png", &w, &h, &c, 4);

	std::cout << "loaded " << w << "x" << h << "\n";

	Image img{};
	glGenTextures(1, &img.tex);
	glBindTexture(GL_TEXTURE_2D, img.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	stbi_image_free(data);

	return img;
}

void draw_rect(Rect &rr, Shader &s, Image &pt, int x, int y, int w, int h, int sw, int sh, Vec3 col) {
	glUseProgram(s.prog);

	glBindTexture(GL_TEXTURE_2D, pt.tex);
	glUniform1i(s.loc_tex, 0);

	glUniform2f(s.loc_pos, (float)x / (float)sw, (float)y / (float)sh);
	glUniform2f(s.loc_scale, (float)w / (float)sw, (float)h / (float)sh);

	glUniform3f(s.loc_color, col.x, col.y, col.z);
	glUniform1f(s.loc_color_fac, 1.f);

	glBindVertexArray(rr.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_piece(Rect &rr, Shader &s, Image &pt, int x, int y, int w, int h, int sw, int sh, colored_piece piece) {
	glUseProgram(s.prog);

	glUniform2f(s.loc_pos, (float)x / (float)sw, (float)y / (float)sh);
	glUniform2f(s.loc_scale, (float)w / (float)sw, (float)h / (float)sh);

	glBindTexture(GL_TEXTURE_2D, pt.tex);
	glUniform1i(s.loc_tex, 0);
	float xoff = (((uint8_t)piece.type()) - 1) * (1.f / 6.f);
	float yoff = (((piece.color())) == piece_color::white) * 1.f / 2.f;
	glUniform2f(s.loc_tex_pos, xoff, yoff);
	glUniform2f(s.loc_tex_scale, 1.f / 6.f, 1.f / 2.f);
	glUniform1f(s.loc_color_fac, 0.f);

	glBindVertexArray(rr.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool key_was_released(const Input &current, int key) {
	bool b = current.key_release[key];
	return b;
}

bool button_was_released(const Input &current, int btn) {
	bool b = current.btn_release[btn];
	return b;
}

void draw_board(Rect &rr, Shader &s, Image &pt, const chess_board &brd, move_list& moves, int offx, int offy, int w, int h, int sw, int sh) {
	auto is_light_square = [](int x, int y) {
		return (x % 2 == 0 && y % 2 == 0) || (x % 2 == 1 && y % 2 == 1);
	};
	auto is_checked_king = [&](int x, int y) {
		if (brd.is_check || brd.is_checkmate) {
			return brd.current_turn == piece_color::white ?
				(brd.white_king_position == x + y * 8) :
				(brd.black_king_position == x + y * 8);
		}
		return false;
	};
	auto is_selected = [&](int x, int y) {
		return x == (brd.selected % 8) && y == (brd.selected / 8);
	};
	auto is_move = [&](int x, int y) {
		for (int i = 0; i < moves.count; i++) {
			if (moves.moves[i] == (x + y * 8)) {
				return true;
			}
		}
		return false;
	};
	auto is_hovered = [&](int x, int y) {
		return brd.hovered_square == (x + y * 8);
	};
	auto get_color = [&](int x, int y) -> Vec3 {
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
		draw_rect(rr, s, pt, px, py, w, h, sw, sh, col);
		if (!brd.get_piece(i).empty())
			draw_piece(rr, s, pt, px, py, w, h, sw, sh, brd.get_piece(i));
	}

	if (brd.wait_for_promotion_selection) {
		// Promotion select bg
		draw_rect(rr, s, pt, 2 * w + offx, 3.5 * h + offy, w * 4, h, sw, sh, { 0.2f, 0.2f, 0.2f });
		// Promotion select highlight
		if (brd.selected != -1) {
			int selection_highlight = brd.selected;
			draw_rect(rr, s, pt, (2 + selection_highlight) * w + offx, 3.5 * h + offy, w, h, sw, sh, { 0.4f, 0.2f, 0.2f });
		}
		piece_color team = brd.current_turn == piece_color::white ? piece_color::black : piece_color::white;
		piece_type protion_pieces[]{ piece_type::queen, piece_type::rook, piece_type::bishop, piece_type::knight };
		for (int a = 0; a < 4; a++) {
			int px = (2 + a) * w + offx;
			int py = 3.5 * h + offy;
			draw_piece(rr, s, pt, px, py, w, h, sw, sh, protion_pieces[a] | team);
		}
	}
}

void process_input(chess_board &brd, move_list& moves, const Input &cin, int sw, int sh) {
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

	moves.count = 0;
	for (int i = 0; i < 64; i++)
		moves.moves[i] = -1;

	if (!brd.is_checkmate) {
		if (!brd.wait_for_promotion_selection) {
			if (button_was_released(cin, GLFW_MOUSE_BUTTON_1)) {
				if (brd.selected == -1) {
					brd.selected = hx + hy * 8;
					// int selected_piece_color = (brd.pieces[brd.selected] & ChessBoard::COLOR_BIT);
					piece_color pieceColor = brd.get_color(brd.selected);
					if (brd.is_empty(brd.selected) || (pieceColor != brd.current_turn)) {
						brd.selected = -1;
					}
				}
				else {
					if (brd.selected == (hx + hy * 8)) {
						brd.selected = -1;
					}
					else if (brd.is_own(brd.selected, hx + hy * 8)) {
						brd.selected = hx + hy * 8;
					}
				}
			}


			if (brd.selected != -1) {
				brd.get_valid_moves(moves, brd.selected);
				if (moves.count == 0) {
					brd.selected = -1;
				}
			}
			else {
				for (int i = 0; i < moves.count; i++) {
					moves.moves[i] = -1;
				}
			}

			// Do the move
			if (button_was_released(cin, GLFW_MOUSE_BUTTON_1)) {
				// Move target
				int move_target = hx + hy * 8;
				if (brd.in_range(move_target, 0, 64)) {
					for (int i = 0; i < moves.count ; i++) {
						if ((moves.moves[i] != -1) && (moves.moves[i] == move_target)) {
							brd.do_move(brd.selected, move_target);
							brd.is_check = false;
							if (brd.is_in_checkmate(brd.current_turn)) {
								brd.is_checkmate = true;
								std::cout << "Checkmate!" << std::endl;
							}
							else if (brd.is_in_check(brd.current_turn)) {
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
				if (button_was_released(cin, GLFW_MOUSE_BUTTON_1)) {
					piece_type promotion_pieces[]{ piece_type::queen, piece_type::rook, piece_type::bishop, piece_type::knight };
					piece_color teamToPromote = brd.current_turn == piece_color::white ? piece_color::black : piece_color::white;
					brd.pieces[brd.to_be_promoted] = promotion_pieces[brd.selected] | teamToPromote;
					brd.to_be_promoted = -1;
					brd.wait_for_promotion_selection = false;
					brd.selected = -1;
					brd.is_check = false;
					if (brd.is_in_checkmate(brd.current_turn)) {
						brd.is_checkmate = true;
						std::cout << "Check mate!" << std::endl;
					}
					else if (brd.is_in_check(brd.current_turn)) {
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

	if (key_was_released(cin, GLFW_KEY_R)) {
		brd.init();
	}
}

void draw_game(Rect &rr, Shader &s, Image &pt, chess_board &brd, move_list& moves, int sw, int sh) {
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
	draw_board(rr, s, pt, brd, moves, offx, offy, w, h, sw, sh);
}
