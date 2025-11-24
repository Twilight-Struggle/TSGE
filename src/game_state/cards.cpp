// どこで: src/game_state/cards.cpp
// 何を: カード実装のエントリーポイント（実装は分離されたファイルに移動）
// なぜ: カード実装をカテゴリごとに整理し、可読性と保守性を向上させるため
//
// すべてのカード実装は以下のファイルに分離されています：
// - src/game_state/cards/scoring_cards.cpp
// - src/game_state/cards/basic_event_cards.cpp
// - src/game_state/cards/special_cards.cpp

#include "tsge/game_state/cards.hpp"
