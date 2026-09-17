#pragma once

#include "board.h"
#include "window.h"

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

Rect create_rect();
Shader create_shader();
Image create_image();

void process_input(chess_board &brd, move_list& moves, const Input &cin, int sw, int sh);
void draw_game(Rect &rr, Shader &s, Image &pt, chess_board &brd, move_list& moves, int sw, int sh);
