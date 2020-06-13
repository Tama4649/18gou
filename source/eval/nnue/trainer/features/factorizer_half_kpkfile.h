// NNUE評価関数の特徴量変換クラステンプレートのHalfKPKfile用特殊化

#ifndef _NNUE_TRAINER_FEATURES_FACTORIZER_HALF_KPKFILE_H_
#define _NNUE_TRAINER_FEATURES_FACTORIZER_HALF_KPKFILE_H_

#include "../../../../config.h"

#if defined(EVAL_NNUE)

#include "../../features/half_kpkfile.h"
#include "../../features/half_kp.h"
#include "../../features/p.h"
#include "../../features/half_relative_kp.h"
#include "factorizer.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 入力特徴量を学習用特徴量に変換するクラステンプレート
// HalfKPKfile用特殊化
template <Side AssociatedKing>
class Factorizer<HalfKPKfile<AssociatedKing>> {
 private:
  using FeatureType = HalfKPKfile<AssociatedKing>;

  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions =
      FeatureType::kMaxActiveDimensions;

  // 学習用特徴量の種類
  enum TrainingFeatureType {
    kFeaturesHalfKPKfile,
    kFeaturesHalfKP,
    kFeaturesHalfK,
    kFeaturesP,
    kFeaturesHalfRelativeKP,
    //
    kNumTrainingFeatureTypes,
  };

  // 学習用特徴量の情報
  static constexpr FeatureProperties kProperties[] = {
    // kFeaturesHalfKPKfile
    {true, FeatureType::kDimensions},
    // kFeaturesHalfKP
    {true, Factorizer<HalfKP<AssociatedKing>>::GetDimensions()},
    // kFeaturesHalfK
    {true, SQ_NB},
    // kFeaturesP
    {true, Factorizer<P>::GetDimensions()},
    // kFeaturesHalfRelativeKP
    {true, Factorizer<HalfRelativeKP<AssociatedKing>>::GetDimensions()},
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
    // kFeaturesHalfKPKfile
    IndexType index_offset = AppendBaseFeature<FeatureType>(
        kProperties[kFeaturesHalfKPKfile], base_index, training_features);

    IndexType index_HalfKP = base_index % (static_cast<IndexType>(fe_end) * static_cast<IndexType>(SQ_NB));
    const auto sq_k = static_cast<Square>(index_HalfKP / fe_end);
    const auto p = static_cast<BonaPiece>(index_HalfKP % fe_end);

    // kFeaturesHalfKP
    index_offset += InheritFeaturesIfRequired<HalfKP<AssociatedKing>>(index_offset, kProperties[kFeaturesHalfKP], index_HalfKP, training_features);

    // kFeaturesHalfK
    {
      const auto& properties = kProperties[kFeaturesHalfK];
      if (properties.active) {
        training_features->emplace_back(index_offset + sq_k);
        index_offset += properties.dimensions;
      }
    }

    // kFeaturesP
    index_offset += InheritFeaturesIfRequired<P>(
        index_offset, kProperties[kFeaturesP], p, training_features);

    // kFeaturesHalfRelativeKP
    if (p >= fe_hand_end) {
      index_offset += InheritFeaturesIfRequired<HalfRelativeKP<AssociatedKing>>(
          index_offset, kProperties[kFeaturesHalfRelativeKP],
          HalfRelativeKP<AssociatedKing>::MakeIndex(sq_k, p),
          training_features);
    } else {
      index_offset += SkipFeatures(kProperties[kFeaturesHalfRelativeKP]);
    }

    ASSERT_LV5(index_offset == GetDimensions());
  }
};

template <Side AssociatedKing>
constexpr FeatureProperties Factorizer<HalfKPKfile<AssociatedKing>>::kProperties[];

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
