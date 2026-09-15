#pragma once

#include <cstdint>
#include <cassert>
#include <string_view>

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
	inline Position(int p) : p(p) { assert(p >= 0); assert(p < 64); }
	inline Position(int p, IgnoreInvalid i) : p(p) {}
	inline Position(int x, int y) : Position(x + y * 8) {}
	inline Position offset(int mv) { return Position(p + mv); }
	
	int p{};

	inline int x() const { return p % 8; }
	inline int y() const { return p / 8; }
	inline bool is_valid() { return (x() >= 0) && (x() < 8) && (y() >= 0) && (y() < 8); }
};

ChessBoard::Color get_color(const ChessBoard &brd, Position p);
ChessBoard::PieceType get_type(const ChessBoard &brd, Position p);
int get_piece(const ChessBoard &brd, Position p);

void init_fen(ChessBoard &brd, std::string_view fen);
void init(ChessBoard &brd);

bool in_range(int val, int min_inc, int max_ex);

bool is_empty(const ChessBoard &brd, Position p);;

bool is_enemy(const ChessBoard &brd, Position self, Position p);;

bool is_enemy_or_empty(const ChessBoard &brd, int self, int p);;

bool is_own(const ChessBoard &brd, int self, int p);;

bool is_in_check(const ChessBoard &brd, ChessBoard::Color c);
void do_move(ChessBoard &brd, Position from_pos, Position to_pos);

bool resolves_check(const ChessBoard &brd, Position pos, Position target);;

bool causes_check_on_self(const ChessBoard &brd, Position pos, Position target);;

void add_move(const ChessBoard &brd, int *move_list, int &move_count, Position pos, int move);;

bool is_valid(Position p, int move);;

void get_pawn_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);

void get_knight_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);;

void get_bishop_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);;
void get_queen_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);

void get_rook_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);

void get_king_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);;

void get_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);;

void get_valid_moves(const ChessBoard &brd, int *move_list, int &move_count, Position p);


bool is_in_checkmate(const ChessBoard &brd, ChessBoard::Color c);

void do_move(ChessBoard &brd, Position from_pos, Position to_pos);

bool resolves_check(const ChessBoard &brd, Position pos, Position target);
