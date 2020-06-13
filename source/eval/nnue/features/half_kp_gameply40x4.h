// NNUE評価関数の入力特徴量HalfKP_GamePly40x4の定義

#ifndef _NNUE_FEATURES_HALF_KP_GAMEPLY40x4_H_
#define _NNUE_FEATURES_HALF_KP_GAMEPLY40x4_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量HalfKP_GamePly40x4：自玉または敵玉の位置と、玉以外の駒の位置と、手数（1～40手目、41～80手目、81～120手目、121手目以降）の組み合わせ
template <Side AssociatedKing>
class HalfKP_GamePly40x4 {
 public:
  // 特徴量名
  static constexpr const char* kName =
      (AssociatedKing == Side::kFriend) ? "HalfKP_GamePly40x4(Friend)" : "HalfKP_GamePly40x4(Enemy)";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue =
      0x1D69D5B9u ^ (AssociatedKing == Side::kFriend);
  // 特徴量の次元数
  static constexpr IndexType kDimensions =
      static_cast<IndexType>(SQ_NB) * static_cast<IndexType>(fe_end) * 4;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = PIECE_NUMBER_KING;
  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger =
      (AssociatedKing == Side::kFriend) ?
      TriggerEvent::kFriendKingMovedOrPly4181121 : TriggerEvent::kEnemyKingMovedOrPly4181121;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // 玉の位置とBonaPieceと手数から特徴量のインデックスを求める
  static IndexType MakeIndex(Square sq_k, BonaPiece p, int ply);

 private:
  // 駒の情報を取得する
  static void GetPieces(const Position& pos, Color perspective,
                        BonaPiece** pieces, Square* sq_target_k);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
