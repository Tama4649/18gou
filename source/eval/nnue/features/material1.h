// NNUE評価関数の入力特徴量Material1の定義

#ifndef _NNUE_FEATURES_MATERIAL1_H_
#define _NNUE_FEATURES_MATERIAL1_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量Material1：駒割り1（盤上の駒と持ち駒を区別しない）
class Material1 {
 public:
  // 特徴量名
  static constexpr const char* kName = "Material1";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue = 0xD3CEE139u;
  // 特徴量の次元数
  static constexpr IndexType kDimensions = 85;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = 13;
  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger = TriggerEvent::kAnyPieceMoved;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // 駒種と枚数から特徴量のインデックスを求める
  static IndexType MakeIndex(Piece pt, int count);

};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
