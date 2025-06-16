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
  EXPECT_TRUE(sut.event(board, Side::USA));
  EXPECT_EQ(board.getVp(), -1);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);
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
  EXPECT_TRUE(sut.event(board, Side::USSR));
  EXPECT_EQ(
      board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USA),
      0);
  EXPECT_EQ(
      board.getWorldMap().getCountry(CountryEnum::CUBA).getInfluence(Side::USSR),
      3);
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
  EXPECT_TRUE(sut.event(board, Side::USSR));
  EXPECT_EQ(board.getVp(), 3);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 5);
  EXPECT_TRUE(board.getDefconTrack().setDefcon(2, board));
  EXPECT_TRUE(sut.event(board, Side::USA));
  EXPECT_EQ(board.getVp(), 3);
  EXPECT_EQ(board.getDefconTrack().getDefcon(), 4);
}