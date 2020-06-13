// NNUE評価関数の入力特徴量KKの定義

#ifndef _NNUE_FEATURES_KK_H_
#define _NNUE_FEATURES_KK_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量KK：自玉の位置と敵玉の位置の組み合わせ
class KK {
 public:
  // 特徴量名
  static constexpr const char* kName = "KK";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue = 0x6C72E637u;
  // 特徴量の次元数
  static constexpr IndexType kDimensions = static_cast<IndexType>(SQ_NB) * static_cast<IndexType>(SQ_NB);
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = 1;
  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger = TriggerEvent::kAnyKingMoved;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // 特徴量のインデックスを求める
  static IndexType MakeIndex(Color perspective, Square ksq1, Square ksq2);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
