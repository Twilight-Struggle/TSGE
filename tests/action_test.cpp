#include "action.hpp"

#include <gtest/gtest.h>

class ActionTest : public ::testing::Test {
 protected:
  Game Game;
};

TEST_F(ActionTest, PlaceTest) {
  // 置く権利のない場所には置けない
  PlaceInfluence action_non_placeable(
      Side::USSR, 3, {{CountryEnum::ANGOLA, 1}, {CountryEnum::AFGHANISTAN, 2}});
  EXPECT_FALSE(action_non_placeable.execute(Game));
  // opeValueと一致しない数置けない
  PlaceInfluence action_opeValue_incorrect(
      Side::USSR, 3,
      {{CountryEnum::AFGHANISTAN, 2}, {CountryEnum::SOUTH_KOREA, 2}});
  EXPECT_FALSE(action_opeValue_incorrect.execute(Game));
  // 相手が支配でoverControlNumが0の場合 false
  PlaceInfluence action_overControlNum_0_false(Side::USA, 1,
                                               {{CountryEnum::NORTH_KOREA, 1}});
  EXPECT_FALSE(action_overControlNum_0_false.execute(Game));
  // 相手が支配でoverControlNumが0の場合 true
  PlaceInfluence action_overControlNum_0_true(Side::USA, 2,
                                              {{CountryEnum::NORTH_KOREA, 1}});
  EXPECT_TRUE(action_overControlNum_0_true.execute(Game));
  // 相手が支配でoverControlNumが1の場合
  PlaceInfluence action_overControlNum_1_prepare(
      Side::USA, 3, {{CountryEnum::SOUTH_KOREA, 3}});
  EXPECT_TRUE(action_overControlNum_1_prepare.execute(Game));
  PlaceInfluence action_overControlNum_1(Side::USSR, 4,
                                         {{CountryEnum::SOUTH_KOREA, 2}});
  EXPECT_TRUE(action_overControlNum_1.execute(Game));
  // 相手が支配でoverControlNumが2の場合
  PlaceInfluence action_overControlNum_2_prepare(Side::USA, 5,
                                                 {{CountryEnum::JAPAN, 5}});
  EXPECT_TRUE(action_overControlNum_2_prepare.execute(Game));
  PlaceInfluence action_overControlNum_2(Side::USSR, 6,
                                         {{CountryEnum::JAPAN, 3}});
  EXPECT_TRUE(action_overControlNum_2.execute(Game));
}

TEST_F(ActionTest, RealigmentTest) {
  // 相手が置いてない国には影響力排除判定ができない
  Realigment action_cant_realigment_ussr(Side::USSR, CountryEnum::ANGOLA);
  EXPECT_FALSE(action_cant_realigment_ussr.execute(Game));
  Realigment action_cant_realigment_usa(Side::USA, CountryEnum::AFGHANISTAN);
  EXPECT_FALSE(action_cant_realigment_usa.execute(Game));
  // 相手が置いている国には影響力排除判定ができる
  Realigment action_can_realigment_ussr(Side::USSR, CountryEnum::SOUTH_KOREA);
  EXPECT_TRUE(action_can_realigment_ussr.execute(Game));
  Realigment action_can_realigment_usa(Side::USA, CountryEnum::NORTH_KOREA);
  EXPECT_TRUE(action_can_realigment_usa.execute(Game));
}

TEST_F(ActionTest, CoupTest) {
  // 相手が置いてない国にはクーデターできない
  Coup action_cant_coup_ussr(Side::USSR, 3, CountryEnum::ANGOLA);
  EXPECT_FALSE(action_cant_coup_ussr.execute(Game));
  Coup action_cant_coup_usa(Side::USA, 3, CountryEnum::AFGHANISTAN);
  EXPECT_FALSE(action_cant_coup_usa.execute(Game));
  // 相手が置いている国にはクーデターできる
  Coup action_can_coup_ussr(Side::USSR, 3, CountryEnum::SOUTH_KOREA);
  EXPECT_TRUE(action_can_coup_ussr.execute(Game));
  Coup action_can_coup_usa(Side::USA, 3, CountryEnum::NORTH_KOREA);
  EXPECT_TRUE(action_can_coup_usa.execute(Game));
}
