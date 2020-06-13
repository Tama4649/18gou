// NNUE評価関数の入力特徴量King24Piece_DistinguishGoldsの定義

#include "../../../config.h"

#if defined(EVAL_NNUE) && defined(LONG_EFFECT_LIBRARY) && defined(DISTINGUISH_GOLDS)

#include "king24_piece_distinguishgolds.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// 盤上の駒のBonaPieceからPieceへの変換配列
extern Piece sqBonaPieceToPiece[];

// BonaPieceからSquareとPieceを取得する
// ・持ち駒の場合は「SQ_NB、NO_PIECE」を返す。本来は持ち駒のPieceを正確に算出することもできるが、この評価関数では不要なので。
template <Side AssociatedKing>
inline void King24Piece_DistinguishGolds<AssociatedKing>::GetSquarePieceFromBonaPiece(BonaPiece bp, Square &sq, Piece &pc) {
  // 持ち駒の場合
  if (bp < fe_hand_end) {
    sq = SQ_NB;
    pc = NO_PIECE;
  }
  // 盤上の駒の場合
  else {
    sq = static_cast<Square>((bp - fe_hand_end) % SQ_NB);
    pc = sqBonaPieceToPiece[(bp - fe_hand_end) / SQ_NB];
  }
}

// Effect24::Directの算出
template <Side AssociatedKing>
inline Effect24::Direct King24Piece_DistinguishGolds<AssociatedKing>::CalcDirect(Square sq_king, Square sq) {
  int file_diff = file_of(sq) - file_of(sq_king);
  int rank_diff = rank_of(sq) - rank_of(sq_king);
  int calc = file_diff * 5 + rank_diff + 12;

  return Effect24::Direct(calc < 12 ? calc : calc - 1);
}

// Pieceの先後反転
template <Side AssociatedKing>
inline Piece King24Piece_DistinguishGolds<AssociatedKing>::Inv(Piece pc) {
  if (pc == NO_PIECE) {
    return NO_PIECE;
  }
  else if (pc == PIECE_WALL) {
    return PIECE_WALL;
  }
  else {
    return make_piece(~color_of(pc), type_of(pc));
  }
}

// Effect24::Directの先後反転
template <Side AssociatedKing>
inline Effect24::Direct King24Piece_DistinguishGolds<AssociatedKing>::Inv(Effect24::Direct dir) {
  return Effect24::DIRECT_NB - static_cast<Effect24::Direct>(1) - dir;
}

// 特徴量のインデックスを求める
template <Side AssociatedKing>
inline IndexType King24Piece_DistinguishGolds<AssociatedKing>::MakeIndex(Color perspective, Effect24::Direct dir, Piece pc) {
  if (perspective == WHITE) {
    pc = Inv(pc);
    dir = Inv(dir);
  }

  return static_cast<IndexType>(dir)
      * static_cast<IndexType>(PIECE_WALL_NB) + static_cast<IndexType>(pc);
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
template <Side AssociatedKing>
void King24Piece_DistinguishGolds<AssociatedKing>::AppendActiveIndices(
    const Position& pos, Color perspective, IndexList* active) {
  // コンパイラの警告を回避するため、配列サイズが小さい場合は何もしない
  if (RawFeatures::kMaxActiveDimensions < kMaxActiveDimensions) return;

  // perspective側の玉のマス（先手目線）
  SquareWithWall sqww_king = to_sqww(pos.king_square(perspective));

  // 24近傍をループ
  for (Effect24::Direct dir : Effect24::Direct()) {
    SquareWithWall sqww = sqww_king + DirectToDeltaWW(dir);

    // 盤内の場合
    if (is_ok(sqww)) {
      Square sq = sqww_to_sq(sqww);
      active->push_back(MakeIndex(perspective, dir, pos.piece_on(sq)));
    }

    // 盤外の場合
    else {
      active->push_back(MakeIndex(perspective, dir, PIECE_WALL));
    }
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
template <Side AssociatedKing>
void King24Piece_DistinguishGolds<AssociatedKing>::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {

  // perspective側の玉のマス（先手目線）
  Square sq_king = pos.king_square(perspective);
  SquareWithWall sqww_king = to_sqww(sq_king);

  // pieces（先手目線）
  BonaPiece* pieces = pos.eval_list()->piece_list_fb();
  const auto& dp = pos.state()->dirtyPiece;

  for (int i = 0; i < dp.dirty_num; ++i) {
    // old_piece（先手目線）
    const auto old_piece = static_cast<BonaPiece>(dp.changed_piece[i].old_piece.from[BLACK]);
    Square old_sq;
    Piece old_pc;
    GetSquarePieceFromBonaPiece(old_piece, old_sq, old_pc);

    // old_pieceのマスが玉の24近傍の場合
    if (old_sq != SQ_NB && dist(sq_king, old_sq) <= 2) {
      Effect24::Direct dir = CalcDirect(sq_king, old_sq);

      removed->push_back(MakeIndex(perspective, dir, old_pc));

      // iが0以外の場合（iが1の場合）は取られた駒なので除外
      if (i == 0) {
        added->push_back(MakeIndex(perspective, dir, NO_PIECE));
      }
    }

    // new_piece（先手目線）
    const auto new_piece = static_cast<BonaPiece>(dp.changed_piece[i].new_piece.from[BLACK]);
    Square new_sq;
    Piece new_pc;
    GetSquarePieceFromBonaPiece(new_piece, new_sq, new_pc);

    // new_pieceのマスが玉の24近傍の場合
    if (new_sq != SQ_NB && dist(sq_king, new_sq) <= 2) {
      Effect24::Direct dir = CalcDirect(sq_king, new_sq);

      // 「dp.dirty_num == 2 && i == 0)」の場合は除外
      if (   (dp.dirty_num == 1 && i == 0)
          || (dp.dirty_num == 2 && i == 1)) {
        removed->push_back(MakeIndex(perspective, dir, NO_PIECE));
      }

      added->push_back(MakeIndex(perspective, dir, new_pc));
    }
  }
}

template class King24Piece_DistinguishGolds<Side::kFriend>;
template class King24Piece_DistinguishGolds<Side::kEnemy>;

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
