// NNUE評価関数の入力特徴量King24AttackEffectの定義

#ifndef _NNUE_FEATURES_KING24_ATTACKEFFECT_H_
#define _NNUE_FEATURES_KING24_ATTACKEFFECT_H_

#include "../../../config.h"

#if defined(EVAL_NNUE) && defined(LONG_EFFECT_LIBRARY)

#include "../../../evaluate.h"
#include "features_common.h"

#include "../../../extra/long_effect.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量King24AttackEffect：玉の24近傍の利き数（攻め方）
template <Side AssociatedKing>
class King24AttackEffect {
 public:
  // 特徴量名
  static constexpr const char* kName =
      (AssociatedKing == Side::kFriend) ? "King24AttackEffect(Friend)" : "King24AttackEffect(Enemy)";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue =
      0x596B2375u ^ (AssociatedKing == Side::kFriend);

  // 特徴量の次元数
  // ・玉の24近傍
  // ・利き数は0～3に制限する
  static constexpr IndexType kDimensions = static_cast<IndexType>(Effect24::DIRECT_NB) * 4;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = Effect24::DIRECT_NB;

  // 差分計算の代わりに全計算を行うタイミング
  static constexpr TriggerEvent kRefreshTrigger =
      (AssociatedKing == Side::kFriend) ?
      TriggerEvent::kFriendKingMoved : TriggerEvent::kEnemyKingMoved;

  // 特徴量のうち、値が1であるインデックスのリストを取得する
  static void AppendActiveIndices(const Position& pos, Color perspective,
                                  IndexList* active);

  // 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
  static void AppendChangedIndices(const Position& pos, Color perspective,
                                   IndexList* removed, IndexList* added);

  // 特徴量のインデックスを求める
  static IndexType MakeIndex(Color perspective, Effect24::Direct dir, int effect);

  // Effect24::Directの先後反転
  static Effect24::Direct Inv(Effect24::Direct dir);

  // BonaPieceからSquareを取得する
  static Square GetSquareFromBonaPiece(BonaPiece bp);

  // Squareのdirty判定
  static bool IsDirty(Square dirty_sq24[], int dirty_sq24_count, Square sq);

  // Effect24::Directの算出
  static Effect24::Direct CalcDirect(Square sq_king, Square sq);

  // 利き数の取得
  static int GetEffectCount(const Position& pos, Square sq, Color perspective, bool prev_effect);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
