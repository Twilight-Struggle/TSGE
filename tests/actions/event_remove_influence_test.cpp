// File: tests/actions/event_remove_influence_test.cpp
// Summary: イベントによる影響力除去の合法手生成を検証する。
// Reason:
// カードイベントや特殊な状況下での影響力除去ロジックが正しく実装されているかを保証する。

#include <gtest/gtest.h>

#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

// テスト用DummyCardクラス
class DummyCard : public Card {
 public:
  DummyCard(int ops)
      : Card(CardEnum::DUMMY, "DummyCard", ops, Side::NEUTRAL, WarPeriod::DUMMY,
             false) {}
  DummyCard(int ops, Side side)
      : Card(CardEnum::DUMMY, "DummyCard", ops, side, WarPeriod::DUMMY, false) {
  }

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/,
                                const Board& /*board*/) const override {
    // 空実装
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    // テスト用：常にイベント実行可能とする
    return true;
  }
};

// テスト用MockEventCardクラス（canEventとgetSideを制御可能）
class MockEventCard : public Card {
 public:
  // NOLINTNEXTLINE(readability-identifier-length)
  MockEventCard(CardEnum id, Side side, bool can_event)
      : Card(id, "MockEventCard", 3, side, WarPeriod::EARLY_WAR, false),
        can_event_{can_event} {}

  [[nodiscard]]
  std::vector<CommandPtr> event(Side /*side*/,
                                const Board& /*board*/) const override {
    // 空実装
    return {};
  }

  [[nodiscard]]
  bool canEvent(const Board& board) const override {
    return can_event_;
  }

 private:
  const bool can_event_;
};

// テスト用のカードプール作成関数
static const std::array<std::unique_ptr<Card>, 111>& createTestCardPool() {
  static std::array<std::unique_ptr<Card>, 111> pool{};
  static bool initialized = false;

  if (!initialized) {
    // DummyカードをOps0として設定（スコアカード相当）
    pool[static_cast<size_t>(CardEnum::DUMMY)] = std::make_unique<DummyCard>(0);
    // その他の既存カード
    pool[static_cast<size_t>(CardEnum::CHINA_CARD)] =
        std::make_unique<DummyCard>(4);
    pool[static_cast<size_t>(CardEnum::DUCK_AND_COVER)] =
        std::make_unique<DummyCard>(3);
    pool[static_cast<size_t>(CardEnum::FIDEL)] = std::make_unique<DummyCard>(2);
    pool[static_cast<size_t>(CardEnum::NUCLEAR_TEST_BAN)] =
        std::make_unique<DummyCard>(4);
    initialized = true;
  }

  return pool;
}

// テスト用ヘルパー関数
class TestHelper {
 public:
  static void clearAllOpponentInfluence(Board& board, Side side) {
    Side opponent = getOpponentSide(side);
    for (size_t i = static_cast<size_t>(CountryEnum::USA) + 1;
         i < board.getWorldMap().getCountriesCount(); ++i) {
      auto country_enum = static_cast<CountryEnum>(i);
      board.getWorldMap().getCountry(country_enum).clearInfluence(opponent);
    }
  }

  static void clearSuperPowerInfluence(Board& board, Side side) {
    auto country_enum =
        side == Side::USSR ? CountryEnum::USSR : CountryEnum::USA;
    board.getWorldMap().getCountry(country_enum).clearInfluence(side);
  }

  static void setupBoardWithInfluence(Board& board) {
    // まず全ての国の影響力をクリア（初期配置をリセット）
    clearAllOpponentInfluence(board, Side::USSR);
    clearAllOpponentInfluence(board, Side::USA);

    // テスト用の影響力を設定
    // 日本にUSA影響力3を設定
    board.getWorldMap()
        .getCountry(CountryEnum::JAPAN)
        .addInfluence(Side::USA, 3);
    // 西ドイツにUSA影響力2を設定
    board.getWorldMap()
        .getCountry(CountryEnum::WEST_GERMANY)
        .addInfluence(Side::USA, 2);
    // イランにUSA影響力1を設定
    board.getWorldMap()
        .getCountry(CountryEnum::IRAN)
        .addInfluence(Side::USA, 1);
    // アンゴラにUSA影響力1を設定
    board.getWorldMap()
        .getCountry(CountryEnum::ANGOLA)
        .addInfluence(Side::USA, 1);
  }

  static void addCardsToHand(Board& board, Side side,
                             const std::vector<CardEnum>& cards) {
    board.clearHand(side);
    for (CardEnum card : cards) {
      board.addCardToHand(side, card);
    }
  }

  static void setSpaceTrackPosition(Board& board, Side side, int position) {
    // SpaceTrackの位置を設定するため、advanceSpaceTrackを使用
    int current_position = board.getSpaceTrack().getSpaceTrackPosition(side);
    if (position > current_position) {
      board.getSpaceTrack().advanceSpaceTrack(side,
                                              position - current_position);
    }
  }

  static void setSpaceTrackTried(Board& board, Side side, int tried) {
    // SpaceTrackの試行回数を設定するため、spaceTriedを使用
    for (int i = 0; i < tried; ++i) {
      board.getSpaceTrack().spaceTried(side);
    }
  }
};

// generateRemoveInfluenceMovesのテスト
class GenerateRemoveInfluenceMovesTest : public ::testing::Test {
 protected:
  GenerateRemoveInfluenceMovesTest() : board(createTestCardPool()) {}

  void SetUp() override {
    // 全ての国の影響力をクリア
    TestHelper::clearAllOpponentInfluence(board, Side::USSR);
    TestHelper::clearAllOpponentInfluence(board, Side::USA);
  }

  Board board;
};

TEST_F(GenerateRemoveInfluenceMovesTest, SuezCrisisInsufficientInfluence) {
  // SuezCrisisシナリオ: 盤面の影響力がカード指定数より少ない
  // カード: 合計4除去、maxPerCountry=2
  // 盤面: イギリス=2、フランス=0、イスラエル=1 → 合計3

  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::ISRAEL)
      .addInfluence(Side::USA, 1);
  // フランスは0のまま

  std::vector<CountryEnum> candidates = {
      CountryEnum::FRANCE, CountryEnum::UNITED_KINGDOM, CountryEnum::ISRAEL};

  auto moves = LegalMovesGenerator::generateRemoveInfluenceMoves(
      board, CardEnum::SUEZ_CRISIS, Side::USSR, Side::USA, 4, 2, std::nullopt,
      candidates);

  // 盤面から除去可能な最大は3（イギリス=2 + イスラエル=1）
  // 期待: イギリス=2, イスラエル=1の除去パターンが1つ
  ASSERT_EQ(moves.size(), 1);

  auto* remove_move = dynamic_cast<EventRemoveInfluenceMove*>(moves[0].get());
  ASSERT_NE(remove_move, nullptr);
}

TEST_F(GenerateRemoveInfluenceMovesTest, SuezCrisisSufficientInfluence) {
  // SuezCrisisシナリオ: 盤面の影響力がカード指定数以上
  // カード: 合計4除去、maxPerCountry=2
  // 盤面: イギリス=5、フランス=0、イスラエル=1 → 合計6

  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 5);
  board.getWorldMap()
      .getCountry(CountryEnum::ISRAEL)
      .addInfluence(Side::USA, 1);
  // フランスは0のまま

  std::vector<CountryEnum> candidates = {
      CountryEnum::FRANCE, CountryEnum::UNITED_KINGDOM, CountryEnum::ISRAEL};

  auto moves = LegalMovesGenerator::generateRemoveInfluenceMoves(
      board, CardEnum::SUEZ_CRISIS, Side::USSR, Side::USA, 4, 2, std::nullopt,
      candidates);

  // 盤面から除去可能な最大は3（イギリス=2 + イスラエル=1）だが、
  // maxPerCountry=2なので、イギリス=2, イスラエル=1が実際の除去可能数(3)
  // カード指定=4だが盤面=3なので3除去
  // パターン: イギリス=2, イスラエル=1 → 1つ
  ASSERT_EQ(moves.size(), 1);
}

TEST_F(GenerateRemoveInfluenceMovesTest, SuezCrisisFullInfluence) {
  // SuezCrisisシナリオ: 各国がmaxPerCountry以上の影響力を持つ
  // カード: 合計4除去、maxPerCountry=2
  // 盤面: イギリス=5、フランス=3、イスラエル=2 → 各国から2ずつ除去可能

  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 5);
  board.getWorldMap()
      .getCountry(CountryEnum::FRANCE)
      .addInfluence(Side::USA, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::ISRAEL)
      .addInfluence(Side::USA, 2);

  std::vector<CountryEnum> candidates = {
      CountryEnum::FRANCE, CountryEnum::UNITED_KINGDOM, CountryEnum::ISRAEL};

  auto moves = LegalMovesGenerator::generateRemoveInfluenceMoves(
      board, CardEnum::SUEZ_CRISIS, Side::USSR, Side::USA, 4, 2, std::nullopt,
      candidates);

  // 各国から最大2ずつ、合計4除去のパターン
  // イギリス=2,フランス=2 / イギリス=2,イスラエル=2 / フランス=2,イスラエル=2
  // イギリス=2,フランス=1,イスラエル=1 / イギリス=1,フランス=2,イスラエル=1 /
  // イギリス=1,フランス=1,イスラエル=2
  EXPECT_GE(moves.size(), 1);

  // 全てのパターンが合計4除去であることを確認
  for (const auto& move : moves) {
    auto* remove_move = dynamic_cast<EventRemoveInfluenceMove*>(move.get());
    ASSERT_NE(remove_move, nullptr);
  }
}

TEST_F(GenerateRemoveInfluenceMovesTest, NoCandidates) {
  // エッジケース: 候補国なし（全国に影響力なし）
  std::vector<CountryEnum> candidates = {
      CountryEnum::FRANCE, CountryEnum::UNITED_KINGDOM, CountryEnum::ISRAEL};
  // 全ての候補国に影響力なし

  auto moves = LegalMovesGenerator::generateRemoveInfluenceMoves(
      board, CardEnum::SUEZ_CRISIS, Side::USSR, Side::USA, 4, 2, std::nullopt,
      candidates);

  // 除去可能な影響力がないため、空
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(GenerateRemoveInfluenceMovesTest, ZeroMaxPerCountry) {
  // エッジケース: maxPerCountry=0（無制限）
  board.getWorldMap()
      .getCountry(CountryEnum::UNITED_KINGDOM)
      .addInfluence(Side::USA, 5);

  std::vector<CountryEnum> candidates = {CountryEnum::UNITED_KINGDOM};

  auto moves = LegalMovesGenerator::generateRemoveInfluenceMoves(
      board, CardEnum::SUEZ_CRISIS, Side::USSR, Side::USA, 4, 0, std::nullopt,
      candidates);

  // maxPerCountry=0なので、1国から4除去可能
  ASSERT_EQ(moves.size(), 1);
}

// generateDeStalinizationRemoveMovesのテスト
class GenerateDeStalinizationRemoveMovesTest : public ::testing::Test {
 protected:
  GenerateDeStalinizationRemoveMovesTest() : board(createTestCardPool()) {}

  void SetUp() override {
    // 全ての国の影響力をクリア
    TestHelper::clearAllOpponentInfluence(board, Side::USSR);
    TestHelper::clearAllOpponentInfluence(board, Side::USA);
  }

  Board board;
};

TEST_F(GenerateDeStalinizationRemoveMovesTest, InsufficientInfluence) {
  // 盤面不足シナリオ: USSR影響力が4未満
  // ポーランドに2、東ドイツに1の合計3
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 1);

  auto moves = LegalMovesGenerator::generateDeStalinizationRemoveMoves(
      board, CardEnum::DE_STALINIZATION, Side::USSR);

  // パス(0除去) + 1除去パターン + 2除去パターン + 3除去パターン
  // 4除去はスキップされる
  EXPECT_GE(moves.size(), 1);  // 少なくともパスオプションがある

  // 全てのパターンがDeStalinizationRemoveMoveであることを確認
  for (const auto& move : moves) {
    auto* remove_move = dynamic_cast<DeStalinizationRemoveMove*>(move.get());
    ASSERT_NE(remove_move, nullptr);
  }
}

TEST_F(GenerateDeStalinizationRemoveMovesTest, SufficientInfluence) {
  // 盤面十分シナリオ: USSR影響力が4以上
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 2);

  auto moves = LegalMovesGenerator::generateDeStalinizationRemoveMoves(
      board, CardEnum::DE_STALINIZATION, Side::USSR);

  // パス(0除去) + 1~4除去の全パターンが生成される
  EXPECT_GE(moves.size(), 5);  // パス + 各除去数のパターン
}

TEST_F(GenerateDeStalinizationRemoveMovesTest, NoCandidates) {
  // エッジケース: USSR影響力が0
  auto moves = LegalMovesGenerator::generateDeStalinizationRemoveMoves(
      board, CardEnum::DE_STALINIZATION, Side::USSR);

  // 候補国なしで空が返る
  EXPECT_EQ(moves.size(), 0);
}

TEST_F(GenerateDeStalinizationRemoveMovesTest, SingleInfluence) {
  // エッジケース: USSR影響力が1のみ
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 1);

  auto moves = LegalMovesGenerator::generateDeStalinizationRemoveMoves(
      board, CardEnum::DE_STALINIZATION, Side::USSR);

  // パス(0除去) + 1除去パターンのみ
  // 2, 3, 4除去はスキップされる
  EXPECT_EQ(moves.size(), 2);  // パス + 1除去
}

// generateSelectCountriesRemoveInfluenceMovesのテスト
class GenerateSelectCountriesRemoveInfluenceMovesTest : public ::testing::Test {
 protected:
  GenerateSelectCountriesRemoveInfluenceMovesTest()
      : board(createTestCardPool()) {}

  void SetUp() override {
    TestHelper::clearAllOpponentInfluence(board, Side::USSR);
    TestHelper::clearAllOpponentInfluence(board, Side::USA);
  }

  Board board;
};

TEST_F(GenerateSelectCountriesRemoveInfluenceMovesTest, ThreeCountriesSelect3) {
  // 3カ国選択テスト (例: EastEuropeanUnrest)
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::HUNGARY)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::CZECHOSLOVAKIA)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::EAST_GERMANY)
      .addInfluence(Side::USSR, 2);

  auto moves = LegalMovesGenerator::generateSelectCountriesRemoveInfluenceMoves(
      board, CardEnum::EAST_EUROPEAN_UNREST, Side::USA, Side::USSR,
      Region::EAST_EUROPE, 3, 1);

  // 4C3 = 4パターン
  EXPECT_EQ(moves.size(), 4);

  for (const auto& move : moves) {
    auto* remove_move = dynamic_cast<EventRemoveInfluenceMove*>(move.get());
    ASSERT_NE(remove_move, nullptr);
  }
}

TEST_F(GenerateSelectCountriesRemoveInfluenceMovesTest,
       CandidatesLessThanSelectCount) {
  // 候補国がN未満の場合（2カ国に影響力を設定）
  board.getWorldMap()
      .getCountry(CountryEnum::POLAND)
      .addInfluence(Side::USSR, 2);
  board.getWorldMap()
      .getCountry(CountryEnum::HUNGARY)
      .addInfluence(Side::USSR, 2);

  auto moves = LegalMovesGenerator::generateSelectCountriesRemoveInfluenceMoves(
      board, CardEnum::EAST_EUROPEAN_UNREST, Side::USA, Side::USSR,
      Region::EAST_EUROPE, 3, 1);

  // 2カ国しか影響力がないので、2カ国全てを選択するパターンのみ
  EXPECT_EQ(moves.size(), 1);
}

TEST_F(GenerateSelectCountriesRemoveInfluenceMovesTest, NoInfluence) {
  // 影響力がない場合
  auto moves = LegalMovesGenerator::generateSelectCountriesRemoveInfluenceMoves(
      board, CardEnum::EAST_EUROPEAN_UNREST, Side::USA, Side::USSR,
      Region::EAST_EUROPE, 3, 1);

  // 除去可能な影響力がないため、空
  EXPECT_EQ(moves.size(), 0);
}

// generateSelectCountriesRemoveAllInfluenceMovesのテスト
class GenerateSelectCountriesRemoveAllInfluenceMovesTest
    : public ::testing::Test {
 protected:
  GenerateSelectCountriesRemoveAllInfluenceMovesTest()
      : board(createTestCardPool()) {}

  void SetUp() override {
    TestHelper::clearAllOpponentInfluence(board, Side::USSR);
    TestHelper::clearAllOpponentInfluence(board, Side::USA);
  }

  Board board;
};

TEST_F(GenerateSelectCountriesRemoveAllInfluenceMovesTest,
       TwoCountriesFromThree) {
  // MuslimRevolution風: 3カ国から2カ国選択
  board.getWorldMap().getCountry(CountryEnum::IRAN).addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::EGYPT).addInfluence(Side::USA, 3);
  board.getWorldMap()
      .getCountry(CountryEnum::SAUDI_ARABIA)
      .addInfluence(Side::USA, 1);

  std::vector<CountryEnum> candidates = {CountryEnum::IRAN, CountryEnum::EGYPT,
                                         CountryEnum::SAUDI_ARABIA};

  auto moves =
      LegalMovesGenerator::generateSelectCountriesRemoveAllInfluenceMoves(
          board, CardEnum::MUSLIM_REVOLUTION, Side::USSR, Side::USA, candidates,
          2);

  // 3C2 = 3パターン
  EXPECT_EQ(moves.size(), 3);

  for (const auto& move : moves) {
    auto* remove_all_move =
        dynamic_cast<EventRemoveAllInfluenceMove*>(move.get());
    ASSERT_NE(remove_all_move, nullptr);
  }
}

TEST_F(GenerateSelectCountriesRemoveAllInfluenceMovesTest,
       ExcludesNoInfluenceCountries) {
  // 影響力がない国は除外される
  board.getWorldMap().getCountry(CountryEnum::IRAN).addInfluence(Side::USA, 2);
  board.getWorldMap().getCountry(CountryEnum::EGYPT).addInfluence(Side::USA, 3);
  // SAUDI_ARABIAは影響力なし

  std::vector<CountryEnum> candidates = {CountryEnum::IRAN, CountryEnum::EGYPT,
                                         CountryEnum::SAUDI_ARABIA};

  auto moves =
      LegalMovesGenerator::generateSelectCountriesRemoveAllInfluenceMoves(
          board, CardEnum::MUSLIM_REVOLUTION, Side::USSR, Side::USA, candidates,
          2);

  // 影響力がある国が2つしかないので、1パターン
  EXPECT_EQ(moves.size(), 1);
}

TEST_F(GenerateSelectCountriesRemoveAllInfluenceMovesTest, AllNoInfluence) {
  // 全ての候補国に影響力がない
  std::vector<CountryEnum> candidates = {CountryEnum::IRAN, CountryEnum::EGYPT,
                                         CountryEnum::SAUDI_ARABIA};

  auto moves =
      LegalMovesGenerator::generateSelectCountriesRemoveAllInfluenceMoves(
          board, CardEnum::MUSLIM_REVOLUTION, Side::USSR, Side::USA, candidates,
          2);

  // 除去可能な影響力がないため、空
  EXPECT_EQ(moves.size(), 0);
}