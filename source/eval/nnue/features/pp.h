// NNUE評価関数の入力特徴量PPの定義

#ifndef _NNUE_FEATURES_PP_H_
#define _NNUE_FEATURES_PP_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量PP：玉以外の駒のBonaPieceの2駒関係
class PP {
 public:
  // 特徴量名
  static constexpr const char* kName = "PP";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue = 0x62A706F2u;
  // 特徴量の次元数
  static constexpr IndexType kDimensions = static_cast<IndexType>(fe_end) * (static_cast<IndexType>(fe_end) - 1) / 2;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = static_cast<IndexType>(PIECE_NUMBER_KING) * (static_cast<IndexType>(PIECE_NUMBER_KING) - 1) / 2;
  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger = TriggerEvent::kNone;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // BonaPieceから特徴量のインデックスを求める
  static IndexType MakeIndex(BonaPiece bp1, BonaPiece bp2);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
