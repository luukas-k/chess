#include "board.h"

ChessBoard::Color get_color(const ChessBoard &brd, Position p) {
	ChessBoard::Color color = (ChessBoard::Color)(brd.pieces[p.p] & ChessBoard::COLOR_BIT);
	return color;
}

ChessBoard::PieceType get_type(const ChessBoard &brd, Position p) {
	ChessBoard::PieceType type = (ChessBoard::PieceType)(brd.pieces[p.p] & ChessBoard::PIECE_BITS);
	return type;
}

int get_piece(const ChessBoard &brd, Position p) {
	return brd.pieces[p.p];
}

void init_fen(ChessBoard &brd, std::string_view fen) {
	brd.selected = -1;
	for (int i = 0; i < 8 * 8; i++) {
		brd.pieces[i] = ChessBoard::None;
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
	char turn = fen[i + 2];
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
	std::string_view castling = fen.substr(i + 2 + 2);
	auto parse_castling = [&](char c) {
		switch (c) {
			case 'K': brd.white_king_side = true; return true;
			case 'Q': brd.white_queen_side = true; return true;
			case 'k': brd.black_king_side = true; return true;
			case 'q': brd.black_queen_side = true; return true;
		}
		assert(false);
		return false;
	};
	brd.black_king_side = false;
	brd.black_queen_side = false;
	brd.white_king_side = false;
	brd.white_queen_side = false;

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
		brd.en_passant_target = x + y * 8;
	}
	else {
		brd.en_passant_target = -1;
	}

	return;
}

void init(ChessBoard &brd) {
	brd = ChessBoard{};
	init_fen(brd, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

int min(int a, int b) {
	return a < b ? a : b;
}

bool in_range(int val, int min_inc, int max_ex) {
	return (val >= min_inc) && (val < max_ex);
}

bool is_empty(const ChessBoard &brd, Position p) {
	return get_type(brd, p) == ChessBoard::None;
}

bool is_enemy(const ChessBoard &brd, Position self, Position p) {
	return get_color(brd, self) != get_color(brd, p) && !is_empty(brd, p);
}

bool is_enemy_or_empty(const ChessBoard &brd, int self, int p) {
	return is_enemy(brd, self, p) || is_empty(brd, p);
}

bool is_own(const ChessBoard &brd, int self, int p) {
	return
		get_color(brd, self) == get_color(brd, p) &&
		get_type(brd, self) != ChessBoard::None &&
		get_type(brd, p) != ChessBoard::None;
}

bool is_in_check(const ChessBoard &brd, ChessBoard::Color c) {
	Position king_location = (c == ChessBoard::White) ? brd.white_king_position : brd.black_king_position;
	ChessBoard::Color opposingColor = (c == ChessBoard::White) ? ChessBoard::Black : ChessBoard::White;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!is_empty(brd, i) && ((get_color(brd, from) == opposingColor))) {
			int piece_move_list[64]{};
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
}

void add_move(const ChessBoard &brd, int *move_list, int &move_count, Position pos, int move) {
	Position targ = pos.offset(move);
	if (!is_empty(brd, targ) && (get_color(brd, pos) == get_color(brd, targ))) {
		return;
	}
	assert(move_count < 64);
	move_list[move_count] = targ.p;
	move_count += 1;
}

bool is_valid(Position p, int move) {
	auto np = Position(p.p + move, IgnoreInvalid::_);
	return np.is_valid();
}

void get_pawn_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
		if (brd.en_passant_target == -1)
			return false;

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
}

void get_knight_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
}

void get_bishop_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
}

void get_queen_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
}

void get_rook_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
}

void get_king_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
	int possible_dirs[]{ Left + Up, Right + Up, Right + Down, Left + Down, Up, Down, Left, Right };
	int possible_dir_count = 8;
	for (int i = 0; i < possible_dir_count; i++) {
		if (is_valid(p, possible_dirs[i])) {
			if (is_empty(brd, p.offset(possible_dirs[i]).p) || (!is_empty(brd, p.offset(possible_dirs[i]).p) && is_enemy(brd, p, p.offset(possible_dirs[i])))) {
				auto target = p.offset(possible_dirs[i]);
				int dx = target.x() > p.x() ? target.x() - p.x() : p.x() - target.x();
				int dy = target.y() > p.y() ? target.y() - p.y() : p.y() - target.y();
				if ((dx == 1 && dy == 0) || (dy == 1 && dx == 0) || (dy == 1 && dx == 1)) {
					add_move(brd, move_list, move_count, p, possible_dirs[i]);
				}
			}
		}
	}

	ChessBoard::Color team = get_color(brd, p);
	int king_row = (team == ChessBoard::White) ? 7 : 0;

	bool can_king_side_castle =
		((team == ChessBoard::White) ? brd.white_king_side : brd.black_king_side) &&
		(p.x() == 4) && (p.y() == king_row) &&
		(get_piece(brd, Position(7, king_row)) == (ChessBoard::Rook | team)) &&
		is_empty(brd, Position(5, king_row)) &&
		is_empty(brd, Position(6, king_row));

	bool can_queen_side_castle =
		((team == ChessBoard::White) ? brd.white_queen_side : brd.black_queen_side) &&
		(p.x() == 4) && (p.y() == king_row) &&
		(get_piece(brd, Position(0, king_row)) == (ChessBoard::Rook | team)) &&
		is_empty(brd, Position(1, king_row)) &&
		is_empty(brd, Position(2, king_row)) &&
		is_empty(brd, Position(3, king_row));

	if (can_queen_side_castle) {
		add_move(brd, move_list, move_count, p, Left * 2);
	}
	if (can_king_side_castle) {
		add_move(brd, move_list, move_count, p, Right * 2);
	}
}

void get_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
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
}

void get_valid_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p) {
	move_count = 0;
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

bool is_in_checkmate(const ChessBoard &brd, ChessBoard::Color c) {
	ChessBoard copy = brd;
	Position king_location = (c == ChessBoard::White) ? brd.white_king_position : brd.black_king_position;
	for (int i = 0; i < 64; i++) {
		Position from(i);
		if (!is_empty(copy, i) && ((get_color(copy, from) == c))) {
			int piece_move_list[64]{};
			int piece_move_count = 0;
			get_valid_moves(copy, piece_move_list, piece_move_count, from);
			if (piece_move_count != 0) {
				return false;
			}
		}
	}
	return true;
}

void do_move(ChessBoard &brd, Position from_pos, Position to_pos) {
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
			int px = to_pos.x();
			int dx = from_pos.x() > px ? from_pos.x() - px : px - from_pos.x();
			if ((brd.en_passant_target + Down * dy == to_pos.p)) {
				// Was en passant so clear en passant target and capture the pawn there
				assert(in_range(brd.en_passant_target, 0, 64));
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

	if (get_type(brd, from_pos) == ChessBoard::King) {
		int dx = to_pos.x() - from_pos.x();
		ChessBoard::Color team = get_color(brd, from_pos);
		int king_row = (team == ChessBoard::White) ? 7 : 0;
		if (dx == 2) {
			brd.pieces[Position(7, king_row).p] = 0;
			brd.pieces[Position(5, king_row).p] = ChessBoard::Rook | team;
			assert(in_range(Position(7, king_row).p, 0, 64));
			assert(in_range(Position(5, king_row).p, 0, 64));
		}
		else if (dx == -2) {
			brd.pieces[Position(0, king_row).p] = 0;
			brd.pieces[Position(3, king_row).p] = ChessBoard::Rook | team;
			assert(in_range(Position(0, king_row).p, 0, 64));
			assert(in_range(Position(3, king_row).p, 0, 64));
		}

		// Update king positions if king was moved
		if (team == ChessBoard::White)
			brd.white_king_position = to_pos.p;
		else
			brd.black_king_position = to_pos.p;
	}

	/* if (brd.pieces[from_pos.p] == (ChessBoard::Color::White | ChessBoard::King)) {
		brd.white_king_position = to_pos.p;
	}
	if (brd.pieces[from_pos.p] == (ChessBoard::Color::Black | ChessBoard::King)) {
		brd.black_king_position = to_pos.p;
	}*/

	assert(in_range(to_pos.p, 0, 64));
	assert(in_range(from_pos.p, 0, 64));
	brd.pieces[to_pos.p] = brd.pieces[from_pos.p];
	brd.pieces[from_pos.p] = 0;

	if (brd.current_turn == ChessBoard::Color::Black) {
		brd.current_turn = ChessBoard::Color::White;
	}
	else {
		brd.current_turn = ChessBoard::Color::Black;
	}
}

bool resolves_check(const ChessBoard &brd, Position pos, Position target) {
	ChessBoard copy = brd;
	do_move(copy, pos, target);
	return !is_in_check(copy, brd.current_turn);
}

bool causes_check_on_self(const ChessBoard &brd, Position pos, Position target) {
	ChessBoard copy = brd;
	do_move(copy, pos, target);
	return is_in_check(copy, brd.current_turn);
}
