// NNUE評価関数の入力特徴量GamePlyの定義

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "gameply.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量のうち、値が1であるインデックスのリストを取得する
void GamePly::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  // 初期局面からの手数（最大255手目）
  active->push_back(std::max(std::min(pos.game_ply(), 255), 0));
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void GamePly::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  // 何もしない（差分計算は未実装）
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
