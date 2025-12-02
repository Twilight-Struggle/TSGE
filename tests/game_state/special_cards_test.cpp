// File: tests/game_state/special_cards_test.cpp
// Summary: 特殊な影響力配置カードのイベント実装を検証する。
// Reason: カードイベントがRequestCommandを正しく生成するかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/command.hpp"
#include "tsge/actions/move.hpp"
#include "tsge/core/board.hpp"
#include "tsge/game_state/cards.hpp"
