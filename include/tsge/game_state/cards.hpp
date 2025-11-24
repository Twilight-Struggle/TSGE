#pragma once
// どこで: include/tsge/game_state/cards.hpp
// 何を: Twilight
// Struggleカード具象クラスを宣言し、Boardへのイベント橋渡しを担う なぜ:
// カードの仕様を型安全に実装し、テスト済みのイベントロジックを共有するため
//
// このファイルは集約ヘッダとして機能し、すべてのカード実装をインクルードする

// スコアリングカード
#include "tsge/game_state/cards/scoring_cards.hpp"

// 基本イベントカード
#include "tsge/game_state/cards/basic_event_cards.hpp"

// 特殊カード
#include "tsge/game_state/cards/special_cards.hpp"
