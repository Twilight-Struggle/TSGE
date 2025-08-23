#include "tsge/core/board.hpp"

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
  auto& deck_cards = deck_.getDeck();
  const int deck_size = static_cast<int>(deck_cards.size());

  if (total_draw_count == 0) {
    return;
  }

  if (total_draw_count < deck_size) {
    for (int i = 0; i < ussrDrawCount; ++i) {
      playerHands_[static_cast<size_t>(Side::USSR)].push_back(
          deck_cards.back());
      deck_cards.pop_back();
    }
    for (int i = 0; i < usaDrawCount; ++i) {
      playerHands_[static_cast<size_t>(Side::USA)].push_back(deck_cards.back());
      deck_cards.pop_back();
    }
  } else if (total_draw_count == deck_size) {
    for (int i = 0; i < ussrDrawCount; ++i) {
      playerHands_[static_cast<size_t>(Side::USSR)].push_back(
          deck_cards.back());
      deck_cards.pop_back();
    }
    for (int i = 0; i < usaDrawCount; ++i) {
      playerHands_[static_cast<size_t>(Side::USA)].push_back(deck_cards.back());
      deck_cards.pop_back();
    }
    deck_.reshuffleFromDiscard();
  } else {
    int ussr_distributed = 0;
    int usa_distributed = 0;

    if (deck_size % 2 == 0) {
      const int cards_per_player = deck_size / 2;
      for (int i = 0; i < cards_per_player && ussr_distributed < ussrDrawCount;
           ++i) {
        playerHands_[static_cast<size_t>(Side::USSR)].push_back(
            deck_cards.back());
        deck_cards.pop_back();
        ussr_distributed++;
      }
      for (int i = 0; i < cards_per_player && usa_distributed < usaDrawCount;
           ++i) {
        playerHands_[static_cast<size_t>(Side::USA)].push_back(
            deck_cards.back());
        deck_cards.pop_back();
        usa_distributed++;
      }
    } else {
      const int base_cards = deck_size / 2;
      for (int i = 0; i < base_cards + 1 && ussr_distributed < ussrDrawCount;
           ++i) {
        playerHands_[static_cast<size_t>(Side::USSR)].push_back(
            deck_cards.back());
        deck_cards.pop_back();
        ussr_distributed++;
      }
      for (int i = 0; i < base_cards && usa_distributed < usaDrawCount; ++i) {
        playerHands_[static_cast<size_t>(Side::USA)].push_back(
            deck_cards.back());
        deck_cards.pop_back();
        usa_distributed++;
      }
    }

    deck_.reshuffleFromDiscard();

    const int ussr_remaining = ussrDrawCount - ussr_distributed;
    const int usa_remaining = usaDrawCount - usa_distributed;

    for (int i = 0; i < ussr_remaining; ++i) {
      playerHands_[static_cast<size_t>(Side::USSR)].push_back(
          deck_cards.back());
      deck_cards.pop_back();
    }
    for (int i = 0; i < usa_remaining; ++i) {
      playerHands_[static_cast<size_t>(Side::USA)].push_back(deck_cards.back());
      deck_cards.pop_back();
    }
  }
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
    card = CardEnum::Dummy;
  }

  // デッキは見えない
  auto& deck = copy.getDeck().getDeck();
  for (auto& card : deck) {
    card = CardEnum::Dummy;
  }

  // ヘッドラインカードの隠蔽
  if (!copy.isHeadlineCardVisible(viewerSide, opponent_side)) {
    copy.headlineCards_[static_cast<size_t>(opponent_side)] = CardEnum::Dummy;
  }

  // Randomizerの外部RNGポインタをリセット（MCTSで独自に設定される）
  // copy.randomizer_.setRng(nullptr);

  return copy;
}