#pragma once

#include <cstdint>
#include <cassert>
#include <string_view>
#include <optional>

enum struct piece_type : uint8_t {
	none = 0,
	king,
	queen,
	bishop,
	knight,
	rook,
	pawn,
};
enum struct piece_color : uint8_t {
	black = 0,
	white = 1 << 4
};

constexpr static uint8_t PIECE_BITS = 0b111;
constexpr static uint8_t COLOR_BIT = 1 << 4;

struct colored_piece {
	inline colored_piece()
		:
		value(0)
	{}
	inline colored_piece(piece_type type, piece_color col)
		:
		value(0)
	{
		value = (uint8_t)type | (uint8_t)col;
	}

	inline bool empty() const {
		return value == 0;
	}

	inline piece_type type() const {
		return (piece_type)(value & PIECE_BITS);
	}
	inline piece_color color() const {
		return (piece_color)(value & COLOR_BIT);
	}

	bool operator==(const colored_piece &rhs) const {
		return value == rhs.value;
	}

	uint8_t value{};
};

inline colored_piece operator|(const piece_type &lhs, const piece_color &rhs) {
	return colored_piece(lhs, rhs);
}

// Directions set to offsets in an array that correspond to movements on the grid
enum {
	Up = 8,
	Down = -8,
	Left = -1,
	Right = 1
};

enum struct IgnoreInvalid { _ };

struct Position {
	inline Position() : p(-1) {}
	inline Position(int p) : p(p) { assert(p >= 0); assert(p < 64); }
	inline Position(int p, IgnoreInvalid i) : p(p) {}
	inline Position(int x, int y) : Position(x + y * 8) {}
	inline Position offset(int mv) { return Position(p + mv); }

	int p{};

	inline int x() const { return p % 8; }
	inline int y() const { return p / 8; }
	inline bool is_valid() { return (x() >= 0) && (x() < 8) && (y() >= 0) && (y() < 8); }
};

struct move_list {
	inline move_list() 
		:
		count(0)
	{}

	// Available moves
	int moves[65]{};
	int count = 0;
};

struct move {
	Position from{}, to{};
	colored_piece capture{};
};

struct chess_board {
	piece_color get_color(Position p);
	piece_type get_type(Position p) const;
	colored_piece get_piece(Position p) const;

	void init_fen(std::string_view fen);
	void init();

	bool is_in_checkmate(piece_color c);
	bool is_in_check(piece_color c);
	void do_move(Position from_pos, Position to_pos);
	bool in_range(int val, int min_inc, int max_ex);
	void get_valid_moves(move_list &mvs, Position p);
	bool is_own(int self, Position p);
	bool is_empty(Position p);

private:
	bool is_enemy(Position self, Position p);;
	bool is_enemy_or_empty(int self, int p);;
	bool resolves_check(Position pos, Position target);;
	bool causes_check_on_self(Position pos, Position target);;
	void add_move(move_list &mvs, Position pos, int move);;
	bool is_valid(Position p, int move);;

	void get_moves(move_list &mvs, Position p);;
	void get_pawn_moves(move_list &mvs, Position p);
	void get_knight_moves(move_list &mvs, Position p);;
	void get_bishop_moves(move_list &mvs, Position p);;
	void get_queen_moves(move_list &mvs, Position p);
	void get_rook_moves(move_list &mvs, Position p);
	void get_king_moves(move_list &mvs, Position p);;

public:

	// Board state
	colored_piece pieces[8 * 8]{};
	piece_color current_turn = piece_color::white;
	// Currently selected piece on the board or in the pawn promotion menu
	int8_t selected{ -1 };
	// Pawn capture information
	int en_passant_target{ -1 };
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


