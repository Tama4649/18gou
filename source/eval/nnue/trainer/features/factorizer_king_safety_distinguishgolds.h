// NNUE評価関数の特徴量変換クラステンプレートのKingSafety_DistinguishGolds用特殊化

#ifndef _NNUE_TRAINER_FEATURES_FACTORIZER_KING_SAFETY_DISTINGUISH_GOLDS_H_
#define _NNUE_TRAINER_FEATURES_FACTORIZER_KING_SAFETY_DISTINGUISH_GOLDS_H_

#include "../../../../config.h"

#if defined(EVAL_NNUE)

#include "../../features/king_safety_distinguishgolds.h"
#include "../../features/king24_piece_distinguishgolds.h"
#include "../../features/king24_defenseeffect.h"
#include "../../features/king24_attackeffect.h"
#include "factorizer.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 入力特徴量を学習用特徴量に変換するクラステンプレート
// KingSafety_DistinguishGolds用特殊化
template <Side AssociatedKing>
class Factorizer<KingSafety_DistinguishGolds<AssociatedKing>> {
 private:
  using FeatureType = KingSafety_DistinguishGolds<AssociatedKing>;

  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions =
      FeatureType::kMaxActiveDimensions;

  // 学習用特徴量の種類
  enum TrainingFeatureType {
    kFeaturesKingSafety_DistinguishGolds,
    kFeaturesKing24Piece_DistinguishGolds,
    kFeaturesKing24DefenseEffect,
    kFeaturesKing24AttackEffect,
    //
    kNumTrainingFeatureTypes,
  };

  // 学習用特徴量の情報
  static constexpr FeatureProperties kProperties[] = {
    // kFeaturesKingSafety_DistinguishGolds
    {true, FeatureType::kDimensions},
    // kFeaturesKing24Piece_DistinguishGolds
    {true, Factorizer<King24Piece_DistinguishGolds<AssociatedKing>>::GetDimensions()},
    // kFeaturesKing24DefenseEffect
    {true, Factorizer<King24DefenseEffect<AssociatedKing>>::GetDimensions()},
    // kFeaturesKing24AttackEffect
    {true, Factorizer<King24AttackEffect<AssociatedKing>>::GetDimensions()},
  };
  static_assert(GetArrayLength(kProperties) == kNumTrainingFeatureTypes, "");

 public:
  // 学習用特徴量の次元数を取得する
  static constexpr IndexType GetDimensions() {
    return GetActiveDimensions(kProperties);
  }

  // 学習用特徴量のインデックスと学習率のスケールを取得する
  static void AppendTrainingFeatures(
      IndexType base_index, std::vector<TrainingFeature>* training_features) {
    // kFeaturesKingSafety_DistinguishGolds
    IndexType index_offset = AppendBaseFeature<FeatureType>(
        kProperties[kFeaturesKingSafety_DistinguishGolds], base_index, training_features);

    const int attackEffect = base_index % 4;
    IndexType tmpIndex = base_index / 4;

    const int defenseEffect = tmpIndex % 4;
    tmpIndex = tmpIndex / 4;

    const auto pc = static_cast<Piece>(tmpIndex % 33);
    const auto dir = static_cast<Effect24::Direct>(tmpIndex / 33);

    // kFeaturesKing24Piece_DistinguishGolds
    index_offset += InheritFeaturesIfRequired<King24Piece_DistinguishGolds<AssociatedKing>>(index_offset, kProperties[kFeaturesKing24Piece_DistinguishGolds], King24Piece_DistinguishGolds<AssociatedKing>::MakeIndex(BLACK, dir, pc), training_features);

    // kFeaturesKing24DefenseEffect
    index_offset += InheritFeaturesIfRequired<King24DefenseEffect<AssociatedKing>>(index_offset, kProperties[kFeaturesKing24DefenseEffect], King24DefenseEffect<AssociatedKing>::MakeIndex(BLACK, dir, defenseEffect), training_features);

    // kFeaturesKing24AttackEffect
    index_offset += InheritFeaturesIfRequired<King24AttackEffect<AssociatedKing>>(index_offset, kProperties[kFeaturesKing24AttackEffect], King24AttackEffect<AssociatedKing>::MakeIndex(BLACK, dir, attackEffect), training_features);

    ASSERT_LV5(index_offset == GetDimensions());
  }
};

template <Side AssociatedKing>
constexpr FeatureProperties Factorizer<KingSafety_DistinguishGolds<AssociatedKing>>::kProperties[];

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
