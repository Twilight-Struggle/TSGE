// どこで: src/game_state/cards/special_cards.cpp
// 何を: 特殊なカードのイベント実装
// なぜ: カード実装を分類し、可読性と保守性を向上させるため
#include "tsge/game_state/cards/special_cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"
