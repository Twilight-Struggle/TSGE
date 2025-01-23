#include "action.hpp"

#include <gtest/gtest.h>

class ActionTest : public ::testing::Test {
 protected:
  Board board;
};

TEST_F(ActionTest, PlaceTest) {
  PlaceInfluence action({{CountryEnum::NORTH_KOREA, 1}});
  action.execute(board, Side::USSR);
  EXPECT_EQ(board.getCountry(CountryEnum::NORTH_KOREA).getInfluence(Side::USSR),
            1);
}