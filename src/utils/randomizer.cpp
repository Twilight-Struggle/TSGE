#include "tsge/utils/randomizer.hpp"

#include <vector>

Randomizer::Randomizer() {
  std::random_device random_device;
  std::vector<std::uint32_t> seed_data(10);
  for (auto& seed : seed_data) {
    seed = random_device();  // ランダムデバイスから値を取得
  }
  // seed_seq を使って mt19937_64 にシードを設定
  std::seed_seq seq(seed_data.begin(), seed_data.end());
  rng = std::mt19937_64(seq);  // 64bit版のメルセンヌ・ツイスタ

  // 一様分布の設定（例：1から100）
  dice = std::uniform_int_distribution<int>(1, 6);
}