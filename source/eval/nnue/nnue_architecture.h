// NNUE評価関数で用いる入力特徴量とネットワーク構造

#ifndef _NNUE_ARCHITECTURE_H_
#define _NNUE_ARCHITECTURE_H_

#include "../../config.h"

#if defined(EVAL_NNUE)

// 入力特徴量とネットワーク構造が定義されたヘッダをincludeする

#if defined(EVAL_NNUE_HALFKPE9)
#include "architectures/halfkpe9_256x2-32-32.h"
#elif defined(EVAL_NNUE_MATERIAL1)
#include "architectures/material1_256x2-32-32.h"
#elif defined(EVAL_NNUE_MATERIAL2)
#include "architectures/material2_256x2-32-32.h"
#elif defined(EVAL_NNUE_MATERIAL2_GAMEPLY)
#include "architectures/material2-gameply_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_KK)
#include "architectures/halfkp-kk_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_PP)
#include "architectures/halfkp-pp_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_GSGS)
#include "architectures/halfkp-gsgs_256x2-32-32.h"

// 金と小駒の成り駒を区別するが、architectureはデフォルトのHalfKPと同じ
#elif defined(EVAL_NNUE_HALFKP_DISTINGUISH_GOLDS)
#include "architectures/halfkp_256x2-32-32.h"

#elif defined(EVAL_NNUE_HALFKP_GAMEPLY40x4)
#include "architectures/halfkp_gameply40x4_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_KINGSAFETY)
#include "architectures/halfkp-kingsafety_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKPKFILE)
#include "architectures/halfkpkfile_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_KINGSAFETY_DISTINGUISH_GOLDS)
#include "architectures/halfkp-kingsafety_distinguishgolds_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKP_KINGSAFETY_DISTINGUISH_GOLDS_FACTORIZER_TEST)
#include "architectures/kingsafety_distinguishgolds_factorizertest_256x2-32-32.h"
#elif defined(EVAL_NNUE_HALFKPE4)
#include "architectures/halfkpe4_256x2-32-32.h"

// KP256型を使いたいときは、これを事前にdefineする。
#elif defined(EVAL_NNUE_KP256)
#include "architectures/k-p_256x2-32-32.h"
#else // #if defined(EVAL_NNUE_HALFKP256)

// NNUE評価関数のデフォルトは、halfKP256
#include "architectures/halfkp_256x2-32-32.h"
#endif

namespace Eval {

namespace NNUE {

static_assert(kTransformedFeatureDimensions % kMaxSimdWidth == 0, "");
static_assert(Network::kOutputDimensions == 1, "");
static_assert(std::is_same<Network::OutputType, std::int32_t>::value, "");

// 差分計算の代わりに全計算を行うタイミングのリスト
constexpr auto kRefreshTriggers = RawFeatures::kRefreshTriggers;

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)

#endif
