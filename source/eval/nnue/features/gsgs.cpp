// NNUE評価関数の入力特徴量GSGSの定義

#include "../../../config.h"

#if defined(EVAL_NNUE)

#include "gsgs.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 一つのBonaPieceのインデックスを求める
inline IndexType GSGS::CalcIndex(BonaPiece bp) {
  // 0～89（実際には金または銀なので59～78）
  if (bp < fe_hand_end) {
    // 0～19
    return bp - f_hand_silver;
  }
  // 90～1547（実際には金または銀なので576～899）
  else {
    // 20～343
    return kNumGSBonaPieceHand + (bp - f_silver);
  }
}

// BonaPieceから特徴量のインデックスを求める
inline IndexType GSGS::MakeIndex(BonaPiece bp1, BonaPiece bp2) {
  IndexType idx1 = CalcIndex(bp1);
  IndexType idx2 = CalcIndex(bp2);

  IndexType idx_max = std::max(idx1, idx2);
  IndexType idx_min = std::min(idx1, idx2);

  return idx_max * (idx_max - 1) / 2 + idx_min;
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
void GSGS::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  const BonaPiece* pieces = (perspective == BLACK) ?
      pos.eval_list()->piece_list_fb() :
      pos.eval_list()->piece_list_fw();

  for (PieceNumber pn1 = PIECE_NUMBER_SILVER; pn1 < PIECE_NUMBER_BISHOP; ++pn1) {
    for (PieceNumber pn2 = PIECE_NUMBER_SILVER; pn2 < pn1; ++pn2) {
      active->push_back(MakeIndex(pieces[pn1], pieces[pn2]));
    }
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
void GSGS::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  const auto& dp = pos.state()->dirtyPiece;

  const BonaPiece* pieces = (perspective == BLACK) ?
      pos.eval_list()->piece_list_fb() :
      pos.eval_list()->piece_list_fw();

  bool dirty_p[2] = { false, false };

  for (int i = 0; i < dp.dirty_num; ++i) {
    if (PIECE_NUMBER_SILVER <= dp.pieceNo[i] && dp.pieceNo[i] < PIECE_NUMBER_BISHOP) {
      dirty_p[i] = true;
    }
  }

  if (dirty_p[0]) {
    if (dirty_p[1]) {
      PieceNumber dirty_pn_p0 = dp.pieceNo[0];
      BonaPiece old_p0 = dp.changed_piece[0].old_piece.from[perspective];
      BonaPiece new_p0 = dp.changed_piece[0].new_piece.from[perspective];

      PieceNumber dirty_pn_p1 = dp.pieceNo[1];
      BonaPiece old_p1 = dp.changed_piece[1].old_piece.from[perspective];
      BonaPiece new_p1 = dp.changed_piece[1].new_piece.from[perspective];

      for (PieceNumber pn_p = PIECE_NUMBER_SILVER; pn_p < PIECE_NUMBER_BISHOP; ++pn_p) {
        if (pn_p != dirty_pn_p0) {
          if (pn_p != dirty_pn_p1) {
            removed->push_back(MakeIndex(pieces[pn_p], old_p0));
            added->push_back(MakeIndex(pieces[pn_p], new_p0));

            removed->push_back(MakeIndex(pieces[pn_p], old_p1));
            added->push_back(MakeIndex(pieces[pn_p], new_p1));
          }
        }
      }

      removed->push_back(MakeIndex(old_p0, old_p1));
      added->push_back(MakeIndex(new_p0, new_p1));
    }
    else {
      PieceNumber dirty_pn_p = dp.pieceNo[0];
      BonaPiece old_p = dp.changed_piece[0].old_piece.from[perspective];
      BonaPiece new_p = dp.changed_piece[0].new_piece.from[perspective];

      for (PieceNumber pn_p = PIECE_NUMBER_SILVER; pn_p < PIECE_NUMBER_BISHOP; ++pn_p) {
        if (pn_p != dirty_pn_p) {
          removed->push_back(MakeIndex(pieces[pn_p], old_p));
          added->push_back(MakeIndex(pieces[pn_p], new_p));
        }
      }
    }
  }
  else {
    if (dirty_p[1]) {
      PieceNumber dirty_pn_p = dp.pieceNo[1];
      BonaPiece old_p = dp.changed_piece[1].old_piece.from[perspective];
      BonaPiece new_p = dp.changed_piece[1].new_piece.from[perspective];

      for (PieceNumber pn_p = PIECE_NUMBER_SILVER; pn_p < PIECE_NUMBER_BISHOP; ++pn_p) {
        if (pn_p != dirty_pn_p) {
          removed->push_back(MakeIndex(pieces[pn_p], old_p));
          added->push_back(MakeIndex(pieces[pn_p], new_p));
        }
      }
    }
  }
}

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
