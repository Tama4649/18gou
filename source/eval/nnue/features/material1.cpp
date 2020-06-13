// NNUE評価関数の入力特徴量Material1の定義

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "material1.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 駒種と枚数から特徴量のインデックスを求める
inline IndexType Material1::MakeIndex(Piece pt, int count) {
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

// 特徴量のうち、値が1であるインデックスのリストを取得する
void Material1::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  // 駒種ごとの枚数
  int pieceTypeCount[PIECE_TYPE_NB] = {};

  // 盤上の駒
  Bitboard bb = pos.pieces(perspective);
  while (bb) {
    Square sq = bb.pop();
    Piece pc = pos.piece_on(sq);
    pieceTypeCount[type_of(pc)]++;
  }

  // 持ち駒
  for (Piece pt = PAWN; pt < PIECE_HAND_NB; ++pt) {
    pieceTypeCount[pt] += hand_count(pos.hand_of(perspective), pt);
  }

  // インデックスリストに追加
  for (Piece pt = PAWN; pt <= DRAGON; ++pt) {
    if (pt != KING) {
      active->push_back(MakeIndex(pt, pieceTypeCount[pt]));
    }
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void Material1::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  // 何もしない（差分計算は未実装）
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
