// NNUE評価関数の入力特徴量King24DefenseEffectの定義

#include "../../../config.h"

#if defined(EVAL_NNUE) && defined(LONG_EFFECT_LIBRARY) && defined(USE_BOARD_EFFECT_PREV)

#include "king24_defenseeffect.h"
#include "index_list.h"

namespace Eval {

namespace NNUE {

namespace Features {

// BonaPieceからSquareを取得する
// ・持ち駒の場合はSQ_NBを返す。
template <Side AssociatedKing>
inline Square King24DefenseEffect<AssociatedKing>::GetSquareFromBonaPiece(BonaPiece bp) {
  // 持ち駒の場合
  if (bp < fe_hand_end) {
    return SQ_NB;
  }
  // 盤上の駒の場合
  else {
    return static_cast<Square>((bp - fe_hand_end) % SQ_NB);
  }
}

// Squareのdirty判定
template <Side AssociatedKing>
inline bool King24DefenseEffect<AssociatedKing>::IsDirty(Square dirty_sq24[], int dirty_sq24_count, Square sq) {
  for (int i = 0; i < dirty_sq24_count; ++i) {
    if (dirty_sq24[i] == sq) {
      return true;
    }
  }
  return false;
}

// Effect24::Directの算出
template <Side AssociatedKing>
inline Effect24::Direct King24DefenseEffect<AssociatedKing>::CalcDirect(Square sq_king, Square sq) {
  int file_diff = file_of(sq) - file_of(sq_king);
  int rank_diff = rank_of(sq) - rank_of(sq_king);
  int calc = file_diff * 5 + rank_diff + 12;

  return Effect24::Direct(calc < 12 ? calc : calc - 1);
}

// 利き数の取得
template <Side AssociatedKing>
inline int King24DefenseEffect<AssociatedKing>::GetEffectCount(const Position& pos, Square sq, Color perspective, bool prev_effect) {
  if (sq == SQ_NB) {
    return 0;
  }
  else {
    if (prev_effect) {
      return std::min(int(pos.board_effect_prev[perspective].effect(sq)), 3);
    }
    else {
      return std::min(int(pos.board_effect[perspective].effect(sq)), 3);
    }
  }
}

// Effect24::Directの先後反転
template <Side AssociatedKing>
inline Effect24::Direct King24DefenseEffect<AssociatedKing>::Inv(Effect24::Direct dir) {
  return Effect24::DIRECT_NB - static_cast<Effect24::Direct>(1) - dir;
}

// 特徴量のインデックスを求める
template <Side AssociatedKing>
inline IndexType King24DefenseEffect<AssociatedKing>::MakeIndex(Color perspective, Effect24::Direct dir, int effect) {
  if (perspective == WHITE) {
    dir = Inv(dir);
  }

  return static_cast<IndexType>(dir)
      * 4 + effect;
}

// 特徴量のうち、値が1であるインデックスのリストを取得する
template <Side AssociatedKing>
void King24DefenseEffect<AssociatedKing>::AppendActiveIndices(
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
      active->push_back(MakeIndex(perspective, dir
          , GetEffectCount(pos, sq, perspective, false)
        ));
    }

    // 盤外の場合
    else {
      // 盤外の場合は追加しない
      //active->push_back(MakeIndex(perspective, dir, 0));
    }
  }
}

// 特徴量のうち、一手前から値が変化したインデックスのリストを取得する
template <Side AssociatedKing>
void King24DefenseEffect<AssociatedKing>::AppendChangedIndices(
    const Position& pos, Color perspective,
    IndexList* removed, IndexList* added) {

  // perspective側の玉のマス（先手目線）
  Square sq_king = pos.king_square(perspective);
  SquareWithWall sqww_king = to_sqww(sq_king);

  // pieces（先手目線）
  BonaPiece* pieces = pos.eval_list()->piece_list_fb();
  const auto& dp = pos.state()->dirtyPiece;

  // 玉の24近傍でdirtyなマス
  Square dirty_sq24[4] = {SQ_NB, SQ_NB, SQ_NB, SQ_NB};
  int dirty_sq24_count = 0;

  for (int i = 0; i < dp.dirty_num; ++i) {
    // old_piece（先手目線）
    const auto old_piece = static_cast<BonaPiece>(dp.changed_piece[i].old_piece.from[BLACK]);
    Square old_sq = GetSquareFromBonaPiece(old_piece);

    // old_pieceのマスが玉の24近傍の場合
    if (old_sq != SQ_NB && dist(sq_king, old_sq) <= 2) {
      dirty_sq24[dirty_sq24_count] = old_sq;
      ++dirty_sq24_count;

      Effect24::Direct dir = CalcDirect(sq_king, old_sq);

      removed->push_back(MakeIndex(perspective, dir
          , GetEffectCount(pos, old_sq, perspective, true)
        ));

      // iが0以外の場合（iが1の場合）は取られた駒なので除外
      if (i == 0) {
        added->push_back(MakeIndex(perspective, dir
            , GetEffectCount(pos, old_sq, perspective, false)
          ));
      }
    }

    // new_piece（先手目線）
    const auto new_piece = static_cast<BonaPiece>(dp.changed_piece[i].new_piece.from[BLACK]);
    Square new_sq = GetSquareFromBonaPiece(new_piece);

    // new_pieceのマスが玉の24近傍の場合
    if (new_sq != SQ_NB && dist(sq_king, new_sq) <= 2) {
      dirty_sq24[dirty_sq24_count] = new_sq;
      ++dirty_sq24_count;

      Effect24::Direct dir = CalcDirect(sq_king, new_sq);

      // 「dp.dirty_num == 2 && i == 0)」の場合は除外
      if (   (dp.dirty_num == 1 && i == 0)
          || (dp.dirty_num == 2 && i == 1)) {
        removed->push_back(MakeIndex(perspective, dir
            , GetEffectCount(pos, new_sq, perspective, true)
          ));
      }

      added->push_back(MakeIndex(perspective, dir
          , GetEffectCount(pos, new_sq, perspective, false)
        ));
    }
  }

  // 24近傍をループ
  for (Effect24::Direct dir : Effect24::Direct()) {
    SquareWithWall sqww = sqww_king + DirectToDeltaWW(dir);

    // 盤内の場合
    if (is_ok(sqww)) {
      Square sq = sqww_to_sq(sqww);

      // dirtyな場合は既に処理済み
      if (IsDirty(dirty_sq24, dirty_sq24_count, sq)) {
        continue;
      }

      int effectCount_prev_1 = GetEffectCount(pos, sq, perspective, true);
      int effectCount_now_1 = GetEffectCount(pos, sq, perspective, false);

      // 利き数に変化があった場合
      if (effectCount_prev_1 != effectCount_now_1) {
        removed->push_back(MakeIndex(perspective, dir, effectCount_prev_1));
        added->push_back(MakeIndex(perspective, dir, effectCount_now_1));
      }
    }

    // 盤外の場合
    else {
      // 何もしない
    }
  }
}

template class King24DefenseEffect<Side::kFriend>;
template class King24DefenseEffect<Side::kEnemy>;

}  // namespace Features

}  // namespace NNUE

}  // namespace Eval

#endif  // defined(EVAL_NNUE)
