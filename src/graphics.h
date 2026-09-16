#pragma once

#include "board.h"

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#else
#include <glad/gl.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <functional>

struct Vec3 {
	float x, y, z;
};

struct Rect {
	uint32_t vao, vbo;
}; 

struct Shader {
	uint32_t prog, vs, fs;

	int
		loc_pos,
		loc_scale,
		loc_tex_pos,
		loc_tex_scale,
		loc_tex,
		loc_color,
		loc_color_fac;
};

struct Image {
	uint32_t tex;
};

struct Input {
	bool keys[256];
	bool btns[256];
	int x, y;
};

Rect create_rect();
Shader create_shader();
Image create_image();

void draw_rect(Rect &rr, Shader &s, Image &pt, int x, int y, int w, int h, int sw, int sh, Vec3 col);
void draw_piece(Rect &rr, Shader &s, Image &pt, int x, int y, int w, int h, int sw, int sh, uint8_t piece);
bool key_was_released(const Input &current, const Input &prev, int key);
bool button_was_released(const Input &current, const Input &prev, int btn);
void draw_board(Rect &rr, Shader &s, Image &pt, const chess_board &brd, move_list& moves, int offx, int offy, int w, int h, int sw, int sh);
void process_input(chess_board &brd, move_list& moves, const Input &cin, const Input &pin, int sw, int sh);
void draw(Rect &rr, Shader &s, Image &pt, chess_board &brd, move_list& moves, int sw, int sh);
