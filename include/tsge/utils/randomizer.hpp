#pragma once

#include <algorithm>
#include <random>
#include <vector>

class Randomizer {
 public:
  Randomizer();
  Randomizer(const Randomizer&) = default;
  Randomizer& operator=(const Randomizer&) = default;
  Randomizer(Randomizer&&) = default;
  Randomizer& operator=(Randomizer&&) = default;
  ~Randomizer() = default;

  // MCTSシミュレーション用
  void setRng(std::mt19937_64* rng) { external_rng_ = rng; }

  int rollDice();

  template <typename T>
  void shuffle(std::vector<T>& vec) {
    auto& rng = (external_rng_ != nullptr) ? *external_rng_ : rng_;
    std::shuffle(vec.begin(), vec.end(), rng);
  }

 private:
  std::mt19937_64 rng_;
  std::mt19937_64* external_rng_ = nullptr;
};