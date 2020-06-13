// NNUE評価関数の入力特徴量HalfKP_GamePly40x4の定義

#include "../../../config.h"

#if defined(EVAL_NNUE) && defined(EVAL_NNUE_HALFKP_GAMEPLY40x4)

#include "half_kp_gameply40x4.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 玉の位置とBonaPieceと手数から特徴量のインデックスを求める
template <Side AssociatedKing>
inline IndexType HalfKP_GamePly40x4<AssociatedKing>::MakeIndex(Square sq_k, BonaPiece p, int ply) {
  int ply_index = std::min(std::max((ply - 1) / 40, 0), 3);
  return static_cast<IndexType>(fe_end) * static_cast<IndexType>(sq_k) + p 
         + ply_index * static_cast<IndexType>(SQ_NB) * static_cast<IndexType>(fe_end);
}

// 駒の情報を取得する
template <Side AssociatedKing>
inline void HalfKP_GamePly40x4<AssociatedKing>::GetPieces(
    const Position& pos, Color perspective,
    BonaPiece** pieces, Square* sq_target_k) {
  *pieces = (perspective == BLACK) ?
      pos.eval_list()->piece_list_fb() :
      pos.eval_list()->piece_list_fw();
  const PieceNumber target = (AssociatedKing == Side::kFriend) ?
      static_cast<PieceNumber>(PIECE_NUMBER_KING + perspective) :
      static_cast<PieceNumber>(PIECE_NUMBER_KING + ~perspective);
  *sq_target_k = static_cast<Square>(((*pieces)[target] - f_king) % SQ_NB);
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
template <Side AssociatedKing>
void HalfKP_GamePly40x4<AssociatedKing>::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  BonaPiece* pieces;
  Square sq_target_k;
  GetPieces(pos, perspective, &pieces, &sq_target_k);
  for (PieceNumber i = PIECE_NUMBER_ZERO; i < PIECE_NUMBER_KING; ++i) {
    active->push_back(MakeIndex(sq_target_k, pieces[i], pos.game_ply()));
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
template <Side AssociatedKing>
void HalfKP_GamePly40x4<AssociatedKing>::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {
  BonaPiece* pieces;
  Square sq_target_k;
  GetPieces(pos, perspective, &pieces, &sq_target_k);
  const auto& dp = pos.state()->dirtyPiece;
  for (int i = 0; i < dp.dirty_num; ++i) {
    if (dp.pieceNo[i] >= PIECE_NUMBER_KING) continue;
    const auto old_p = static_cast<BonaPiece>(
        dp.changed_piece[i].old_piece.from[perspective]);
    removed->push_back(MakeIndex(sq_target_k, old_p, pos.game_ply() - 1));
    const auto new_p = static_cast<BonaPiece>(
        dp.changed_piece[i].new_piece.from[perspective]);
    added->push_back(MakeIndex(sq_target_k, new_p, pos.game_ply()));
  }
}

template class HalfKP_GamePly40x4<Side::kFriend>;
template class HalfKP_GamePly40x4<Side::kEnemy>;

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
