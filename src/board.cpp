#include "board.h"

piece_color chess_board::get_color(Position p) {
	/*piece_color color = (piece_color)(pieces[p.p] & COLOR_BIT);
	return color;*/
	return pieces[p.p].color();
}

piece_type chess_board::get_type(Position p) const {
	/*piece_type type = (piece_type)(pieces[p.p] & PIECE_BITS);
	return type;*/
	return pieces[p.p].type();
}

colored_piece chess_board::get_piece(Position p) const {
	return pieces[p.p];
}

void chess_board::init_fen(std::string_view fen) {
	selected = -1;
	for (int i = 0; i < 8 * 8; i++) {
		pieces[i] = colored_piece();
	}
	size_t len = fen.size();
	size_t i = 0;
	int cursor = 0;
	// Board state
	for (; i < len; i++) {
		char c = fen[i];
		// 'PNBRQK'
		if (i <= 64) {
			if (c == 'P') {
				pieces[cursor++] = piece_type::pawn | piece_color::white;
			}
			else if (c == 'N') {
				pieces[cursor++] = piece_type::knight | piece_color::white;
			}
			else if (c == 'B') {
				pieces[cursor++] = piece_type::bishop | piece_color::white;
			}
			else if (c == 'R') {
				pieces[cursor++] = piece_type::rook | piece_color::white;
			}
			else if (c == 'Q') {
				pieces[cursor++] = piece_type::queen | piece_color::white;
			}
			else if (c == 'K') {
				white_king_position = cursor;
				pieces[cursor++] = piece_type::king | piece_color::white;
			}
			else if (c == 'p') {
				pieces[cursor++] = piece_type::pawn | piece_color::black;
			}
			else if (c == 'n') {
				pieces[cursor++] = piece_type::knight | piece_color::black;
			}
			else if (c == 'b') {
				pieces[cursor++] = piece_type::bishop | piece_color::black;
			}
			else if (c == 'r') {
				pieces[cursor++] = piece_type::rook | piece_color::black;
			}
			else if (c == 'q') {
				pieces[cursor++] = piece_type::queen | piece_color::black;
			}
			else if (c == 'k') {
				black_king_position = cursor;
				pieces[cursor++] = piece_type::king | piece_color::black;
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
	char turn = fen[i + 2];
	if (turn == 'w') {
		current_turn = piece_color::white;
	}
	else if (turn == 'b') {
		current_turn = piece_color::black;
	}
	else {
		assert(false);
	}
	// Castling availability
	std::string_view castling = fen.substr(i + 2 + 2);
	auto parse_castling = [&](char c) {
		switch (c) {
			case 'K': white_king_side = true; return true;
			case 'Q': white_queen_side = true; return true;
			case 'k': black_king_side = true; return true;
			case 'q': black_queen_side = true; return true;
		}
		assert(false);
		return false;
	};
	black_king_side = false;
	black_queen_side = false;
	white_king_side = false;
	white_queen_side = false;

	std::string_view en_passant_target = castling.substr(1);
	if (castling[0] == '-') {} // No castling
	else if (parse_castling(castling[0]) &&
			 castling[1] == ' ') {
		en_passant_target = en_passant_target.substr(1);
	}
	else if (parse_castling(castling[0]) &&
			 parse_castling(castling[1]) &&
			 castling[2] == ' ') {
		en_passant_target = en_passant_target.substr(2);
	}
	else if (parse_castling(castling[0]) &&
			 parse_castling(castling[1]) &&
			 parse_castling(castling[2]) &&
			 castling[3] == ' ') {
		en_passant_target = en_passant_target.substr(3);
	}
	else if (parse_castling(castling[0]) &&
			 parse_castling(castling[1]) &&
			 parse_castling(castling[2]) &&
			 parse_castling(castling[3]) &&
			 castling[4] == ' ') {
		en_passant_target = en_passant_target.substr(4);
	}
	else {
		assert(false);
	}

	if (en_passant_target[0] != '-') {
		char x = en_passant_target[0] - 'a';
		char y = en_passant_target[1] - '1';
		this->en_passant_target = x + y * 8;
	}
	else {
		this->en_passant_target = -1;
	}

	return;
}

void chess_board::init() {
	*this = chess_board{}; // TODO: ?
	init_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

int min(int a, int b) {
	return a < b ? a : b;
}

bool chess_board::in_range(int val, int min_inc, int max_ex) {
	return (val >= min_inc) && (val < max_ex);
}

bool chess_board::is_empty(Position p) {
	return get_piece(p).empty();
}

bool chess_board::is_enemy(Position self, Position p) {
	return get_color(self) != get_color(p) && !is_empty(p);
}

bool chess_board::is_enemy_or_empty(int self, int p) {
	return is_enemy(self, p) || is_empty(p);
}

bool chess_board::is_own(int self, int p) {
	return
		get_color(self) == get_color(p) &&
		get_type(self) != piece_type::none &&
		get_type(p) != piece_type::none;
}

bool chess_board::is_in_check(piece_color c) {
	Position king_location = (c == piece_color::white) ? white_king_position : black_king_position;
	piece_color opposingColor = (c == piece_color::white) ? piece_color::black : piece_color::white;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!is_empty(i) && ((get_color(from) == opposingColor))) {
			int piece_move_list[64]{};
			int piece_move_count = 0;
			get_moves(piece_move_list, piece_move_count, from);
			for (int mv_i = 0; mv_i < piece_move_count; mv_i++) {
				Position target(piece_move_list[mv_i]);
				if (target.p == king_location.p) {
					return true;
				}
			}
		}
	}
	return false;
}

void chess_board::add_move(int *move_list, int &move_count, Position pos, int move) {
	Position targ = pos.offset(move);
	if (!is_empty(targ) && (get_color(pos) == get_color(targ))) {
		return;
	}
	assert(move_count < 64);
	move_list[move_count] = targ.p;
	move_count += 1;
}

bool chess_board::is_valid(Position p, int move) {
	auto np = Position(p.p + move, IgnoreInvalid::_);
	return np.is_valid();
}

void chess_board::get_pawn_moves(int *move_list, int &move_count, Position p) {
	int dir =
		((pieces[(int)p.p].color()) == piece_color::black) ?
		Up : Down;
	bool can_move = (dir == Up) ? (p.y() != 7) : (p.y() != 0);
	bool can_double_move = (dir == Up) ? (p.y() == 1) : (p.y() == 6);

	if (!can_move) {
		return;
	}
	if (is_empty(p.offset(dir))) {
		add_move(move_list, move_count, p, dir);
		if (can_double_move && is_empty(p.offset(dir * 2))) {
			add_move(move_list, move_count, p, dir * 2);
		}
	}
	auto can_enpassant = [&](Position from, bool towards_left) {
		if (en_passant_target == -1)
			return false;

		Position enemy_pawn = from.offset(towards_left ? Left : Right);
		if (towards_left) {
			if (enemy_pawn.x() == from.x() - 1) {
				if (en_passant_target == enemy_pawn.p) {
					return true;
				}
			}
		}
		else {
			if (enemy_pawn.x() == from.x() + 1) {
				if (en_passant_target == enemy_pawn.p) {
					return true;
				}
			}
		}
		return false;
	};
	if (p.x() != 0) {
		if (!is_empty(p.offset(dir + Left)) && is_enemy(p, p.offset(dir + Left))) {
			add_move(move_list, move_count, p, dir + Left);
		}
		if (can_enpassant(p, true)) {
			add_move(move_list, move_count, p, dir + Left);
		}
	}
	if (p.x() != 7) {
		if (!is_empty(p.offset(dir + Right)) && is_enemy(p, p.offset(dir + Right))) {
			add_move(move_list, move_count, p, dir + Right);
		}
		if (can_enpassant(p, false)) {
			add_move(move_list, move_count, p, dir + Right);
		}
	}
}

void chess_board::get_knight_moves(int *move_list, int &move_count, Position p) {
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
			if (is_empty(p.offset(possible_moves[i]).p) || (!is_empty(p.offset(possible_moves[i]).p) && is_enemy(p, p.offset(possible_moves[i]))))
				add_move(move_list, move_count, p, possible_moves[i]);
		}
	}
}

void chess_board::get_bishop_moves(int *move_list, int &move_count, Position p) {
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
				if (!is_empty(tp.p) && is_enemy(p, tp)) {
					add_move(move_list, move_count, p, tp.p - p.p);
					break;
				}
				if (is_own(p.p, tp.p)) {
					break;
				}
				add_move(move_list, move_count, p, tp.p - p.p);
			}
		}
	}
}

void chess_board::get_queen_moves(int *move_list, int &move_count, Position p) {
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
				if (!is_empty(p.p + possible_dirs[i] * (dist + 1)) && is_enemy(p, p.p + possible_dirs[i] * (dist + 1))) {
					add_move(move_list, move_count, p, possible_dirs[i] * (dist + 1));
					break;
				}
				if (is_own(p.p, p.p + possible_dirs[i] * (dist + 1))) {
					break;
				}
				add_move(move_list, move_count, p, possible_dirs[i] * (dist + 1));
			}
		}
	}
}

void chess_board::get_rook_moves(int *move_list, int &move_count, Position p) {
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
				if (!is_empty(p.p + possible_dirs[i] * (dist + 1)) && is_enemy(p, p.p + possible_dirs[i] * (dist + 1))) {
					add_move(move_list, move_count, p, possible_dirs[i] * (dist + 1));
					break;
				}
				if (is_own(p.p, p.p + possible_dirs[i] * (dist + 1))) {
					break;
				}
				add_move(move_list, move_count, p, possible_dirs[i] * (dist + 1));
			}
		}
	}
}

void chess_board::get_king_moves(int *move_list, int &move_count, Position p) {
	int possible_dirs[]{ Left + Up, Right + Up, Right + Down, Left + Down, Up, Down, Left, Right };
	int possible_dir_count = 8;
	for (int i = 0; i < possible_dir_count; i++) {
		if (is_valid(p, possible_dirs[i])) {
			if (is_empty(p.offset(possible_dirs[i]).p) || (!is_empty(p.offset(possible_dirs[i]).p) && is_enemy(p, p.offset(possible_dirs[i])))) {
				auto target = p.offset(possible_dirs[i]);
				int dx = target.x() > p.x() ? target.x() - p.x() : p.x() - target.x();
				int dy = target.y() > p.y() ? target.y() - p.y() : p.y() - target.y();
				if ((dx == 1 && dy == 0) || (dy == 1 && dx == 0) || (dy == 1 && dx == 1)) {
					add_move(move_list, move_count, p, possible_dirs[i]);
				}
			}
		}
	}

	piece_color team = get_color(p);
	int king_row = (team == piece_color::white) ? 7 : 0;

	bool can_king_side_castle =
		((team == piece_color::white) ? white_king_side : black_king_side) &&
		(p.x() == 4) && (p.y() == king_row) &&
		(get_piece(Position(7, king_row)) == (piece_type::rook | team)) &&
		is_empty(Position(5, king_row)) &&
		is_empty(Position(6, king_row));

	bool can_queen_side_castle =
		((team == piece_color::white) ? white_queen_side : black_queen_side) &&
		(p.x() == 4) && (p.y() == king_row) &&
		(get_piece( Position(0, king_row)) == (piece_type::rook | team)) &&
		is_empty(Position(1, king_row)) &&
		is_empty(Position(2, king_row)) &&
		is_empty(Position(3, king_row));

	if (can_queen_side_castle) {
		add_move(move_list, move_count, p, Left * 2);
	}
	if (can_king_side_castle) {
		add_move(move_list, move_count, p, Right * 2);
	}
}

void chess_board::get_moves(int *move_list, int &move_count, Position p) {
	auto type = get_type(p);
	if (type == piece_type::pawn)
		get_pawn_moves(move_list, move_count, p);
	else if (type == piece_type::knight)
		get_knight_moves(move_list, move_count, p);
	else if (type == piece_type::bishop)
		get_bishop_moves(move_list, move_count, p);
	else if (type == piece_type::queen)
		get_queen_moves(move_list, move_count, p);
	else if (type == piece_type::king)
		get_king_moves(move_list, move_count, p);
	else if (type == piece_type::rook)
		get_rook_moves(move_list, move_count, p);
	else {
		move_count = 0;
	}
}

void chess_board::get_valid_moves(int *move_list, int &move_count, Position p) {
	move_count = 0;
	get_moves(move_list, move_count, p);
	int valid_move_count = 0;
	for (int mv_i = 0; mv_i < move_count; mv_i++) {
		if (is_check) {
			if (resolves_check(p, move_list[mv_i])) {
				move_list[valid_move_count] = move_list[mv_i];
				valid_move_count++;
			}
		}
		else {
			if (!causes_check_on_self(p, move_list[mv_i])) {
				move_list[valid_move_count] = move_list[mv_i];
				valid_move_count++;
			}
		}
	}
	move_count = valid_move_count;
}

bool chess_board::is_in_checkmate(piece_color c) {
	chess_board copy = *this;
	Position king_location = (c == piece_color::white) ? white_king_position : black_king_position;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!copy.is_empty(i) && ((copy.get_color(from) == c))) {
			int piece_move_list[64]{};
			int piece_move_count = 0;
			copy.get_valid_moves(piece_move_list, piece_move_count, from);
			if (piece_move_count != 0) {
				return false;
			}
		}
	}
	return true;
}

void chess_board::do_move(Position from_pos, Position to_pos) {
	if (from_pos.p == to_pos.p)
		return;
	// for (int k = 0; k < 64; k++) {
		// Check if valid move
		// if (brd.highlights[k] != -1 && brd.highlights[k] == to_pos) {
			// if (in_range(brd.selected, 0, 64)) {
				// Was valid so do the move
	if (get_type(from_pos) == piece_type::pawn) {
		// Was a pawn
		int py = to_pos.y();
		int dy = from_pos.y() > py ? from_pos.y() - py : py - from_pos.y();
		if (dy == 2) {
			// Was double move so update en passant target
			en_passant_target = to_pos.p;
		}
		else {
			// Was single move so check wether it was en passant capture
			int px = to_pos.x();
			int dx = from_pos.x() > px ? from_pos.x() - px : px - from_pos.x();
			if ((en_passant_target + Down * dy == to_pos.p)) {
				// Was en passant so clear en passant target and capture the pawn there
				assert(in_range(en_passant_target, 0, 64));
				pieces[en_passant_target] = colored_piece();
				en_passant_target = -1;
			}
			// Wasn't double move so clear en passant target
			en_passant_target = -1;
		}
		if ((to_pos.y() == 7) || (to_pos.y() == 0)) {
			to_be_promoted = to_pos.p;
			wait_for_promotion_selection = true;
		}
	}
	else {
		en_passant_target = -1;
	}

	if (get_type(from_pos) == piece_type::king) {
		int dx = to_pos.x() - from_pos.x();
		piece_color team = get_color(from_pos);
		int king_row = (team == piece_color::white) ? 7 : 0;
		if (dx == 2) {
			pieces[Position(7, king_row).p] = colored_piece();
			pieces[Position(5, king_row).p] = piece_type::rook | team;
			assert(in_range(Position(7, king_row).p, 0, 64));
			assert(in_range(Position(5, king_row).p, 0, 64));
		}
		else if (dx == -2) {
			pieces[Position(0, king_row).p] = colored_piece();
			pieces[Position(3, king_row).p] = piece_type::rook | team;
			assert(in_range(Position(0, king_row).p, 0, 64));
			assert(in_range(Position(3, king_row).p, 0, 64));
		}

		// Update king positions if king was moved
		if (team == piece_color::white)
			white_king_position = to_pos.p;
		else
			black_king_position = to_pos.p;
	}

	/* if (brd.pieces[from_pos.p] == (ChessBoard::Color::White | ChessBoard::King)) {
		brd.white_king_position = to_pos.p;
	}
	if (brd.pieces[from_pos.p] == (ChessBoard::Color::Black | ChessBoard::King)) {
		brd.black_king_position = to_pos.p;
	}*/

	assert(in_range(to_pos.p, 0, 64));
	assert(in_range(from_pos.p, 0, 64));
	pieces[to_pos.p] = pieces[from_pos.p];
	pieces[from_pos.p] = colored_piece();

	if (current_turn == piece_color::black) {
		current_turn = piece_color::white;
	}
	else {
		current_turn = piece_color::black;
	}
}

bool chess_board::resolves_check(Position pos, Position target) {
	chess_board copy = *this;
	copy.do_move(pos, target);
	return !copy.is_in_check(current_turn);
}

bool chess_board::causes_check_on_self(Position pos, Position target) {
	chess_board copy = *this;
	copy.do_move(pos, target);
	return copy.is_in_check(current_turn);
}
