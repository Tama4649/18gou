// NNUE評価関数の入力特徴量KingSafetyの定義

#include "../../../config.h"

#if defined(EVAL_NNUE) && defined(LONG_EFFECT_LIBRARY)

#include "king_safety.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// Pieceの先後反転
inline Piece KingSafety::Inv(Piece pc) {
  if (pc == NO_PIECE) {
    return NO_PIECE;
  }
  else if (pc == PIECE_WALL) {
    return PIECE_WALL;
  }
  else {
    return make_piece(~color_of(pc), type_of(pc));
  }
}

// Effect24::Directの先後反転
inline Effect24::Direct KingSafety::Inv(Effect24::Direct dir) {
  return Effect24::DIRECT_NB - static_cast<Effect24::Direct>(1) - dir;
}

// 特徴量のインデックスを求める
inline IndexType KingSafety::MakeIndex(Color perspective, Effect24::Direct dir, Piece pc, int effect1, int effect2) {
  if (perspective == WHITE) {
    pc = Inv(pc);
    dir = Inv(dir);
  }

  return ((static_cast<IndexType>(dir)
      * static_cast<IndexType>(PIECE_WALL_NB) + static_cast<IndexType>(pc))
      * 4 + std::min(effect1, 3))
      * 4 + std::min(effect2, 3);
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
void KingSafety::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  auto& board_effect = pos.board_effect;
  SquareWithWall sqww_king = to_sqww(pos.king_square(perspective));

  for (Effect24::Direct dir : Effect24::Direct()) {
    SquareWithWall sqww = sqww_king + DirectToDeltaWW(dir);

    if (is_ok(sqww)) {
      Square sq = sqww_to_sq(sqww);
      active->push_back(MakeIndex(perspective, dir, pos.piece_on(sq), board_effect[perspective].effect(sq), board_effect[~perspective].effect(sq)));
    }
    else {
      active->push_back(MakeIndex(perspective, dir, PIECE_WALL, 0, 0));
    }
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void KingSafety::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  // 差分計算なし
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
