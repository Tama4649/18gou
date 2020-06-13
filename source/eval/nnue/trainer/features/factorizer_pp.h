// NNUE評価関数の特徴量変換クラステンプレートのPP用特殊化

#ifndef _NNUE_TRAINER_FEATURES_FACTORIZER_PP_H_
#define _NNUE_TRAINER_FEATURES_FACTORIZER_PP_H_

#include "../../../../config.h"

#if defined(EVAL_NNUE)

#include "../../features/pp.h"
#include "../../features/p.h"
#include "factorizer.h"

namespace Eval {

namespace NNUE {

// フィボナッチ数列
int fibonacci[fe_end + 1];

namespace Features {

// 入力特徴量を学習用特徴量に変換するクラステンプレート
// PP用特殊化
template <>
class Factorizer<PP> {
 private:
  using FeatureType = PP;

  // 特徴量のうち、同時に値が1となるインデックスの数の最大値
  static constexpr IndexType kMaxActiveDimensions =
      FeatureType::kMaxActiveDimensions;

  // 学習用特徴量の種類
  enum TrainingFeatureType {
    kFeaturesPP,
    kFeaturesP1,
    kFeaturesP2,
    kNumTrainingFeatureTypes,
  };

  // 学習用特徴量の情報
  static constexpr FeatureProperties kProperties[] = {
    // kFeaturesPP
    {true, FeatureType::kDimensions},
    // kFeaturesP1
    {true, Factorizer<P>::GetDimensions()},
    // kFeaturesP2
    {true, Factorizer<P>::GetDimensions()},
  };
  static_assert(GetArrayLength(kProperties) == kNumTrainingFeatureTypes, "");

  // PPのインデックスから2つのPのインデックスを取得
  static void GetP1P2Index(IndexType base_index, IndexType& p1, IndexType& p2) {
    for (int i = sqrt(base_index); true; i++) {
      if ((int)base_index < fibonacci[i]) {
        p1 = i;
        p2 = i - (fibonacci[i] - base_index);
        return;
      }
    }
  }

 public:
  // 学習用特徴量の次元数を取得する
  static constexpr IndexType GetDimensions() {
    return GetActiveDimensions(kProperties);
  }

  // 学習用特徴量のインデックスと学習率のスケールを取得する
  static void AppendTrainingFeatures(
      IndexType base_index, std::vector<TrainingFeature>* training_features) {
    // kFeaturesPP
    IndexType index_offset = AppendBaseFeature<FeatureType>(
        kProperties[kFeaturesPP], base_index, training_features);

    // PPのインデックスから2つのPのインデックスを取得
    IndexType p1, p2;
    GetP1P2Index(base_index, p1, p2);

    // kFeaturesP1
    index_offset += InheritFeaturesIfRequired<P>(index_offset, kProperties[kFeaturesP1], p1, training_features);
    // kFeaturesP2
    index_offset += InheritFeaturesIfRequired<P>(index_offset, kProperties[kFeaturesP2], p2, training_features);

    ASSERT_LV5(index_offset == GetDimensions());
  }
};

template <>
constexpr FeatureProperties Factorizer<PP>::kProperties[];

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
