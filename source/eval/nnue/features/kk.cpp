// NNUE評価関数の入力特徴量KKの定義

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "kk.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量のインデックスを求める
inline IndexType KK::MakeIndex(Color perspective, Square ksq1, Square ksq2) {
  if (perspective == WHITE) {
    ksq1 = Inv(ksq1);
    ksq2 = Inv(ksq2);
  }

  return static_cast<IndexType>(ksq1) * static_cast<IndexType>(SQ_NB) + ksq2;
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
void KK::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  active->push_back(MakeIndex(perspective, pos.king_square(perspective), pos.king_square(~perspective)));
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void KK::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  // 何もしない
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
