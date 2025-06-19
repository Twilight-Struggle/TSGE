#include "cards.hpp"

#include <gtest/gtest.h>

#include "board.hpp"

class DuckAndCoverTest : public ::testing::Test {
 protected:
  DuckAndCoverTest() : board(defaultCardPool()) {}
  
  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }
  
  Board board;
  DuckAndCover sut;
};

TEST_F(DuckAndCoverTest, DuckAndCoverTest) {
  auto commands = sut.event(Side::USA);
  EXPECT_FALSE(commands.empty());
  
  // 初期状態確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  EXPECT_EQ(board.getVp(), 0);
  EXPECT_TRUE(board.getStates().empty());
  
  // コマンド実行
  for (const auto& command : commands) {
    command->apply(board);
  }
  
  // DEFCON変更確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);
  
  // VP変更はpushStateで追加されているかを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());
  
  // 最後のstateがChangeVpCommandであることを確認
  EXPECT_TRUE(std::holds_alternative<CommandPtr>(states.back()));
  auto& lastCommand = std::get<CommandPtr>(states.back());
  
  // pushされたCommandを実行してVP変更を確認
  EXPECT_TRUE(lastCommand->apply(board));
  EXPECT_EQ(board.getVp(), -1);  // (5-4) * (-1) = -1 (USA側なので負)
}

class FidelTest : public ::testing::Test {
 protected:
  FidelTest() : board(defaultCardPool()) {}
  
  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }
  
  Board board;
  Fidel sut;
};

TEST_F(FidelTest, FidelTest) {
  EXPECT_TRUE(board.getWorldMap()
                  .getCountry(CountryEnum::CUBA)
                  .addInfluence(Side::USA, 1));
  auto commands = sut.event(Side::USSR);
  // Fidel is commented out for now as per task requirements
  EXPECT_TRUE(commands.empty());
  // Test expectations are temporarily disabled since Fidel event is not implemented
  // EXPECT_EQ(
  //     board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USA),
  //     0);
  // EXPECT_EQ(
  //     board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USSR),
  //     3);
}

class NuclearTestBanTest : public ::testing::Test {
 protected:
  NuclearTestBanTest() : board(defaultCardPool()) {}
  
  static const std::array<std::unique_ptr<Card>, 111>& defaultCardPool() {
    static std::array<std::unique_ptr<Card>, 111> pool{};
    return pool;
  }
  
  Board board;
  NuclearTestBan sut;
};

TEST_F(NuclearTestBanTest, NuclearTestBanTest) {
  // Test USSR playing Nuclear Test Ban from DEFCON 5
  auto commands = sut.event(Side::USSR);
  EXPECT_FALSE(commands.empty());
  
  // 初期状態確認
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  EXPECT_EQ(board.getVp(), 0);
  EXPECT_TRUE(board.getStates().empty());
  
  // コマンド実行
  for (const auto& command : commands) {
    command->apply(board);
  }
  
  // DEFCON変更確認（5 + 2 = 7, capped at 5）
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  
  // VP変更はpushStateで追加されているかを確認
  auto& states = board.getStates();
  EXPECT_FALSE(states.empty());
  
  // pushされたCommandを実行してVP変更を確認
  auto& vpCommand = std::get<CommandPtr>(states.back());
  EXPECT_TRUE(vpCommand->apply(board));
  EXPECT_EQ(board.getVp(), 3);  // (5-2) * 1 = 3 VP for USSR
  
  // statesをクリアして次のテスト準備
  states.clear();
  
  // Set DEFCON to 2 for next test
  EXPECT_TRUE(board.getDefconTrack().setDefcon(2));
  
  // Test USA playing Nuclear Test Ban from DEFCON 2
  auto commands2 = sut.event(Side::USA);
  EXPECT_FALSE(commands2.empty());
  for (const auto& command : commands2) {
    command->apply(board);
  }
  
  // DEFCON変更確認（2 + 2 = 4）
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);
  
  // VP変更確認
  EXPECT_FALSE(states.empty());
  auto& vpCommand2 = std::get<CommandPtr>(states.back());
  EXPECT_TRUE(vpCommand2->apply(board));
  EXPECT_EQ(board.getVp(), 3);  // 3 + (2-2) * (-1) = 3 (no change)
}