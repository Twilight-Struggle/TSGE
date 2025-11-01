#include "tsge/core/board.hpp"

void Board::giveChinaCardTo(Side newOwner, bool faceUp) {
  if (newOwner != Side::USSR && newOwner != Side::USA) [[unlikely]] {
    return;
  }
  chinaCard_.owner = newOwner;
  chinaCard_.faceUp = faceUp;
}

void Board::revealChinaCard() {
  if (chinaCard_.owner == Side::NEUTRAL) [[unlikely]] {
    return;
  }
  chinaCard_.faceUp = true;
}

void Board::finalScoring() {
  int final_score = 0;
  final_score += getVpMultiplier(chinaCard_.owner) * 1;
  pushState(std::make_shared<ChangeVpCommand>(Side::USSR, final_score));
}

std::array<int, 2> Board::calculateDrawCount(int turn) const {
  const int required_cards = actionRoundTrack_.getDefinedActionRounds(turn) + 2;

  const int ussr_current_cards =
      static_cast<int>(playerHands_[static_cast<size_t>(Side::USSR)].size());
  const int usa_current_cards =
      static_cast<int>(playerHands_[static_cast<size_t>(Side::USA)].size());

  const int ussr_draw_count = std::max(0, required_cards - ussr_current_cards);
  const int usa_draw_count = std::max(0, required_cards - usa_current_cards);

  return {ussr_draw_count, usa_draw_count};
}

void Board::drawCardsForPlayers(int ussrDrawCount, int usaDrawCount) {
  const int total_draw_count = ussrDrawCount + usaDrawCount;
  if (total_draw_count == 0) {
    return;
  }

  auto& deck_cards = deck_.getDeck();
  const int deck_size = static_cast<int>(deck_cards.size());

  const auto draw_for_side = [&](Side side, int count) {
    auto& hand = playerHands_[static_cast<size_t>(side)];
    for (int i = 0; i < count; ++i) {
      hand.push_back(deck_cards.back());
      deck_cards.pop_back();
    }
  };

  const auto draw_limited_for_side = [&](Side side, int& remaining, int cap) {
    const int draw_cap = std::min(remaining, cap);
    const int draw_count =
        std::min(draw_cap, static_cast<int>(deck_cards.size()));
    draw_for_side(side, draw_count);
    remaining -= draw_count;
  };

  if (total_draw_count <= deck_size) {
    draw_for_side(Side::USSR, ussrDrawCount);
    draw_for_side(Side::USA, usaDrawCount);
    if (total_draw_count == deck_size) {
      deck_.reshuffleFromDiscard();
    }
    return;
  }

  int ussr_remaining = ussrDrawCount;
  int usa_remaining = usaDrawCount;

  const int ussr_share = (deck_size + 1) / 2;
  const int usa_share = deck_size - ussr_share;

  draw_limited_for_side(Side::USSR, ussr_remaining, ussr_share);
  draw_limited_for_side(Side::USA, usa_remaining, usa_share);

  deck_.reshuffleFromDiscard();

  draw_limited_for_side(Side::USSR, ussr_remaining, ussr_remaining);
  draw_limited_for_side(Side::USA, usa_remaining, usa_remaining);
}

bool Board::isHeadlineCardVisible(Side viewer, Side target) const {
  // 自分のカードは常に見える
  if (viewer == target) {
    return true;
  }

  // 宇宙開発トラック4の優位性チェック
  // viewerがトラック4以上でtargetがトラック4未満の場合、targetのカードが見える
  return spaceTrack_.effectEnabled(viewer, 4);
}

Board Board::copyForMCTS(Side viewerSide) const {
  // shared_ptrに変更したことで、完全なコピーコンストラクタが使用可能
  Board copy = *this;

  // 相手側のSideを取得
  const Side opponent_side = getOpponentSide(viewerSide);

  // 相手の手札を隠蔽（カード枚数は維持し、内容をDummyに置換）
  // TODO: 将来activeEvents_メンバを追加して、CIA Createdなどのイベントによる
  // 手札可視性の変更に対応する（例：activeEvents_にCIA_Createdが含まれている場合は隠蔽しない）
  auto& opponent_hand = copy.playerHands_[static_cast<size_t>(opponent_side)];
  for (auto& card : opponent_hand) {
    card = CardEnum::DUMMY;
  }

  // デッキは見えない
  auto& deck = copy.getDeck().getDeck();
  for (auto& card : deck) {
    card = CardEnum::DUMMY;
  }

  // ヘッドラインカードの隠蔽
  if (!copy.isHeadlineCardVisible(viewerSide, opponent_side)) {
    copy.headlineCards_[static_cast<size_t>(opponent_side)] = CardEnum::DUMMY;
  }

  // Randomizerの外部RNGポインタをリセット（MCTSで独自に設定される）
  // copy.randomizer_.setRng(nullptr);

  return copy;
}
