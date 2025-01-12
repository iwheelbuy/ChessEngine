/*
 Stockfish, a UCI chess playing engine derived from Glaurung 2.1
 Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
 Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
 Copyright (C) 2015-2017 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad
 
 Stockfish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Stockfish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>

#include "types.h"

Value PieceValue[PHASE_NB][PIECE_NB] = {
   { VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg },
   { VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg }
};

namespace PSQT {

#define S(mg, eg) make_score(mg, eg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
const Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
   { },
   { // Pawn
      { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
      { S(-11, 7), S(  6,-4), S(  7, 8), S( 3,-2) },
      { S(-18,-4), S( -2,-5), S( 19, 5), S(24, 4) },
      { S(-17, 3), S( -9, 3), S( 20,-8), S(35,-3) },
      { S( -6, 8), S(  5, 9), S(  3, 7), S(21,-6) },
      { S( -6, 8), S( -8,-5), S( -6, 2), S(-2, 4) },
      { S( -4, 3), S( 20,-9), S( -8, 1), S(-4,18) }
   },
   { // Knight
      { S(-143, -97), S(-96,-82), S(-80,-46), S(-73,-14) },
      { S( -83, -69), S(-43,-55), S(-21,-17), S(-10,  9) },
      { S( -71, -50), S(-22,-39), S(  0, -8), S(  9, 28) },
      { S( -25, -41), S( 18,-25), S( 43,  7), S( 47, 38) },
      { S( -26, -46), S( 16,-25), S( 38,  2), S( 50, 41) },
      { S( -11, -55), S( 37,-38), S( 56, -8), S( 71, 27) },
      { S( -62, -64), S(-17,-50), S(  5,-24), S( 14, 13) },
      { S(-195,-110), S(-66,-90), S(-42,-50), S(-29,-13) }
   },
   { // Bishop
      { S(-54,-68), S(-23,-40), S(-35,-46), S(-44,-28) },
      { S(-30,-43), S( 10,-17), S(  2,-23), S( -9, -5) },
      { S(-19,-32), S( 17, -9), S( 11,-13), S(  1,  8) },
      { S(-21,-36), S( 18,-13), S( 11,-15), S(  0,  7) },
      { S(-21,-36), S( 14,-14), S(  6,-17), S( -1,  3) },
      { S(-27,-35), S(  6,-13), S(  2,-10), S( -8,  1) },
      { S(-33,-44), S(  7,-21), S( -4,-22), S(-12, -4) },
      { S(-45,-65), S(-21,-42), S(-29,-46), S(-39,-27) }
   },
   { // Rook
      { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
      { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
      { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
      { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
      { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
      { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
      { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
      { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
   },
   { // Queen
      { S( 0,-70), S(-3,-57), S(-4,-41), S(-1,-29) },
      { S(-4,-58), S( 6,-30), S( 9,-21), S( 8, -4) },
      { S(-2,-39), S( 6,-17), S( 9, -7), S( 9,  5) },
      { S(-1,-29), S( 8, -5), S(10,  9), S( 7, 17) },
      { S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 23) },
      { S(-2,-40), S( 6,-16), S( 8,-11), S(10,  3) },
      { S(-2,-54), S( 7,-30), S( 7,-21), S( 6, -7) },
      { S(-1,-75), S(-4,-54), S(-1,-44), S( 0,-30) }
   },
   { // King
      { S(260,  0), S(313, 48), S(263, 75), S(188, 84) },
      { S(258, 42), S(298, 91), S(232,142), S(174,131) },
      { S(195, 81), S(240,136), S(171,167), S(105,163) },
      { S(173,103), S(181,166), S(144,166), S(106,176) },
      { S(146,104), S(174,159), S(112,196), S( 63,199) },
      { S(116, 90), S(157,150), S( 82,171), S( 39,169) },
      { S( 85, 44), S(127, 93), S( 62,114), S( 17,133) },
      { S( 63,  2), S( 89, 48), S( 47, 73), S(  0, 83) }
   }
};

#undef S

Score psq[PIECE_NB][SQUARE_NB];

// init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] adding the piece value, then the black halves of the
// tables are initialized by flipping and changing the sign of the white scores.
void init() {
   
   for (Piece pc = W_PAWN; pc <= W_KING; ++pc)
   {
      PieceValue[MG][~pc] = PieceValue[MG][pc];
      PieceValue[EG][~pc] = PieceValue[EG][pc];
      
      Score v = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);
      
      for (Square s = SQ_A1; s <= SQ_H8; ++s)
      {
         File f = std::min(file_of(s), FILE_H - file_of(s));
         psq[ pc][ s] = v + Bonus[pc][rank_of(s)][f];
         psq[~pc][~s] = -psq[pc][s];
      }
   }
}

} // namespace PSQT
