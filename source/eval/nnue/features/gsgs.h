// NNUE評価関数の入力特徴量GSGSの定義

#ifndef _NNUE_FEATURES_GSGS_H_
#define _NNUE_FEATURES_GSGS_H_

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "../../../evaluate.h"
#include "features_common.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 特徴量GSGS：「金と銀」のBonaPieceの2駒関係
class GSGS {
 public:
  // 特徴量名
  static constexpr const char* kName = "GSGS";
  // 評価関数ファイルに埋め込むハッシュ値
  static constexpr std::uint32_t kHashValue = 0x28C35A27u;

  // 「金と銀」の枚数
  static constexpr IndexType kNumGSPieces = PIECE_NUMBER_BISHOP - PIECE_NUMBER_SILVER;

  // 「金と銀」のBonaPiece数（持ち駒、盤上の駒）
  static constexpr IndexType kNumGSBonaPieceHand = f_hand_bishop - f_hand_silver;
  static constexpr IndexType kNumGSBonaPieceSq = f_bishop - f_silver;
  static constexpr IndexType kNumGSBonaPiece = kNumGSBonaPieceHand + kNumGSBonaPieceSq;

  // 特徴量の次元数
  static constexpr IndexType kDimensions = kNumGSBonaPiece * (kNumGSBonaPiece - 1) / 2;
  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions = kNumGSPieces * (kNumGSPieces - 1) / 2;
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

  // 一つのBonaPieceのインデックスを求める
  static IndexType CalcIndex(BonaPiece bp);
};

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
