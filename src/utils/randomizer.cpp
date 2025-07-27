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
  rng_ = std::mt19937_64(seq);  // 64bit版のメルセンヌ・ツイスタ
}

int Randomizer::rollDice() {
  std::uniform_int_distribution<int> dice(1, 6);
  auto& rng = (external_rng_ != nullptr) ? *external_rng_ : rng_;
  return dice(rng);
}