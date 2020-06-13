// NNUE評価関数の入力特徴量Material2の定義

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "material2.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 駒種と枚数から特徴量のインデックスを求める（盤上の駒）
inline IndexType Material2::MakeIndex_Square(Piece pt, int count) {
  switch (pt) {
    case PAWN:          return  0 + count;
    case LANCE:         return 19 + count;
    case KNIGHT:        return 24 + count;
    case SILVER:        return 29 + count;
    case GOLD:          return 34 + count;
    case BISHOP:        return 39 + count;
    case ROOK:          return 42 + count;
    case PRO_PAWN:      return 45 + count;
    case PRO_LANCE:     return 64 + count;
    case PRO_KNIGHT:    return 69 + count;
    case PRO_SILVER:    return 74 + count;
    case HORSE:         return 79 + count;
    case DRAGON:        return 82 + count;
    default:            return  0;
  }
}

// 駒種と枚数から特徴量のインデックスを求める（持ち駒）
inline IndexType Material2::MakeIndex_Hand(Piece pt, int count) {
  switch (pt) {
    case PAWN:          return  85 + count;
    case LANCE:         return 104 + count;
    case KNIGHT:        return 109 + count;
    case SILVER:        return 114 + count;
    case GOLD:          return 119 + count;
    case BISHOP:        return 124 + count;
    case ROOK:          return 127 + count;
    default:            return   0;
  }
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
void Material2::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  // 駒種ごとの枚数（盤上の駒）
  int pieceTypeCount_Square[PIECE_TYPE_NB] = {};
  // 駒種ごとの枚数（持ち駒）
  int pieceTypeCount_Hand[PIECE_HAND_NB] = {};

  // 盤上の駒
  Bitboard bb = pos.pieces(perspective);
  while (bb) {
    Square sq = bb.pop();
    Piece pc = pos.piece_on(sq);
    pieceTypeCount_Square[type_of(pc)]++;
  }

  // 持ち駒
  for (Piece pt = PAWN; pt < PIECE_HAND_NB; ++pt) {
    pieceTypeCount_Hand[pt] = hand_count(pos.hand_of(perspective), pt);
  }

  // インデックスリストに追加（盤上の駒）
  for (Piece pt = PAWN; pt <= DRAGON; ++pt) {
    if (pt != KING) {
      active->push_back(MakeIndex_Square(pt, pieceTypeCount_Square[pt]));
    }
  }

  // インデックスリストに追加（持ち駒）
  for (Piece pt = PAWN; pt < PIECE_HAND_NB; ++pt) {
    active->push_back(MakeIndex_Hand(pt, pieceTypeCount_Hand[pt]));
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void Material2::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  // 何もしない（差分計算は未実装）
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
