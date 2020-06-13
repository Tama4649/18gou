// NNUE評価関数の入力特徴量HalfKPKfileの定義

#ifndef _NNUE_FEATURES_HALF_KPKFILE_H_
#define _NNUE_FEATURES_HALF_KPKFILE_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量HalfKPKfile：自玉または敵玉の位置と、玉以外の駒の位置と、他方の玉の位置の筋（File）の組み合わせ
// ・普通にHalfKKPにすると評価関数ファイルのサイズがHalfKPの81倍と巨大になるので、他方の玉の位置は筋のみに絞る（段は使用しない）ことで9倍に抑える。
template <Side AssociatedKing>
class HalfKPKfile {
 public:
  // 特徴量名
  static constexpr const char* kName =
      (AssociatedKing == Side::kFriend) ? "HalfKPKfile(Friend)" : "HalfKPKfile(Enemy)";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue =
      0x5D69D5B9u ^ (AssociatedKing == Side::kFriend);
  // 特徴量の次元数
  static constexpr IndexType kDimensions =
      static_cast<IndexType>(SQ_NB) * static_cast<IndexType>(fe_end) * 9;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = PIECE_NUMBER_KING;
  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger = TriggerEvent::kAnyKingMoved;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // 玉の位置とBonaPieceから特徴量のインデックスを求める
  static IndexType MakeIndex(Square sq_k, BonaPiece p, Square sq_other_k);

 private:
  // 駒の情報を取得する
  static void GetPieces(const Position& pos, Color perspective,
                        BonaPiece** pieces, Square* sq_target_k, Square* sq_other_k);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
