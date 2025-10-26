#pragma once
#include <map>
#include <memory>
#include <vector>

#include "tsge/actions/command.hpp"
#include "tsge/enums/cards_enum.hpp"
#include "tsge/enums/game_enums.hpp"
#include "tsge/game_state/card.hpp"

enum class AdditionalOpsType : uint8_t {
  NONE = 0,
  CHINA_CARD = 1 << 0,
  VIETNAM_REVOLTS = 1 << 1,
  BOTH = CHINA_CARD | VIETNAM_REVOLTS
};

class Game;

class Move {
 public:
  Move(CardEnum card, Side side) : card_{card}, side_{side} {}
  virtual ~Move() = default;
  Move(const Move&) = delete;
  Move& operator=(const Move&) = delete;
  Move(Move&&) = delete;
  Move& operator=(Move&&) = delete;

  [[nodiscard]]
  CardEnum getCard() const {
    return card_;
  }
  [[nodiscard]]
  Side getSide() const {
    return side_;
  }
  [[nodiscard]]
  virtual std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const = 0;

  [[nodiscard]]
  virtual bool operator==(const Move& other) const = 0;

 private:
  const CardEnum card_;
  const Side side_;
};

class ActionPlaceInfluenceMove final : public Move {
 public:
  ActionPlaceInfluenceMove(CardEnum card, Side side,
                           const std::map<CountryEnum, int>& targetCountries)
      : Move{card, side}, targetCountries_{targetCountries} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast =
        dynamic_cast<const ActionPlaceInfluenceMove*>(&other);
    if (other_cast == nullptr) {
      return false;
    }
    return targetCountries_ == other_cast->targetCountries_;
  }

 private:
  const std::map<CountryEnum, int> targetCountries_;
};

class ActionCoupMove final : public Move {
 public:
  ActionCoupMove(CardEnum card, Side side, CountryEnum targetCountry)
      : Move{card, side}, targetCountry_{targetCountry} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const ActionCoupMove*>(&other);
    if (other_cast == nullptr) {
      return false;
    }
    return targetCountry_ == other_cast->targetCountry_;
  }

 private:
  const CountryEnum targetCountry_;
};

class ActionSpaceRaceMove final : public Move {
 public:
  ActionSpaceRaceMove(CardEnum card, Side side) : Move{card, side} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const ActionSpaceRaceMove*>(&other);
    return other_cast != nullptr;
  }
};

class ActionRealigmentMove final : public Move {
 public:
  ActionRealigmentMove(CardEnum card, Side side, CountryEnum targetCountry)
      : Move{card, side}, targetCountry_{targetCountry} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const ActionRealigmentMove*>(&other);
    if (other_cast == nullptr) {
      return false;
    }
    return targetCountry_ == other_cast->targetCountry_;
  }

 private:
  const CountryEnum targetCountry_;
};

// Request内部で使用される、追加のRequestを生成しないRealignmentMove
class RealignmentRequestMove final : public Move {
 public:
  RealignmentRequestMove(
      CardEnum card, Side side, CountryEnum targetCountry,
      const std::vector<CountryEnum>& history, int remainingOps,
      AdditionalOpsType appliedAdditionalOps = AdditionalOpsType::NONE)
      : Move{card, side},
        targetCountry_{targetCountry},
        realignmentHistory_{history},
        remainingOps_{remainingOps},
        appliedAdditionalOps_{appliedAdditionalOps} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast =
        dynamic_cast<const RealignmentRequestMove*>(&other);
    if (other_cast == nullptr) {
      return false;
    }
    return targetCountry_ == other_cast->targetCountry_ &&
           realignmentHistory_ == other_cast->realignmentHistory_ &&
           remainingOps_ == other_cast->remainingOps_ &&
           appliedAdditionalOps_ == other_cast->appliedAdditionalOps_;
  }

 private:
  const CountryEnum targetCountry_;
  const std::vector<CountryEnum> realignmentHistory_;
  const int remainingOps_;
  const AdditionalOpsType appliedAdditionalOps_;
};

class ActionEventMove final : public Move {
 public:
  ActionEventMove(CardEnum card, Side side) : Move{card, side} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    if (this->getCard() != other.getCard() ||
        this->getSide() != other.getSide()) {
      return false;
    }
    const auto* other_cast = dynamic_cast<const ActionEventMove*>(&other);
    return other_cast != nullptr;
  }
};

// 追加AR専用のパスムーブ。カードを消費せず即座にAR完了処理へ遷移する。
class ExtraActionPassMove final : public Move {
 public:
  explicit ExtraActionPassMove(Side side) : Move{CardEnum::Dummy, side} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& /*card*/) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    return this->getSide() == other.getSide() &&
           dynamic_cast<const ExtraActionPassMove*>(&other) != nullptr;
  }
};

class HeadlineCardSelectMove final : public Move {
 public:
  HeadlineCardSelectMove(CardEnum card, Side side) : Move{card, side} {}

  [[nodiscard]]
  std::vector<CommandPtr> toCommand(
      const std::unique_ptr<Card>& card) const override;

  [[nodiscard]]
  bool operator==(const Move& other) const override {
    return this->getCard() == other.getCard() &&
           this->getSide() == other.getSide() &&
           dynamic_cast<const HeadlineCardSelectMove*>(&other) != nullptr;
  }
};
