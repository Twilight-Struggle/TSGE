// どこで: src/game_state/cards/basic_event_cards.cpp
// 何を: 基本的なイベントカードのイベント実装
// なぜ: カード実装を分類し、可読性と保守性を向上させるため
#include "tsge/game_state/cards/basic_event_cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

std::vector<CommandPtr> DuckAndCover::event(Side side,
                                            const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<ChangeDefconCommand>(-1));
  commands.push_back(std::make_shared<LambdaCommand>([](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (5 - current_defcon);
    board.pushState(std::make_shared<ChangeVpCommand>(Side::USA, vp_change));
  }));
  return commands;
}

bool DuckAndCover::canEvent(const Board& board) const {
  return true;
}

std::vector<CommandPtr> ChinaCard::event(Side /*side*/,
                                         const Board& /*board*/) const {
  return {};
}

bool ChinaCard::canEvent(const Board& /*board*/) const {
  return false;
}

std::vector<CommandPtr> Fidel::event(Side side, const Board& /*board*/) const {
  // auto& cuba = board.getWorldMap().getCountry(CountryEnum::CUBA);
  // cuba.clearInfluence(Side::USA);
  // auto sufficientInfluence =
  //     cuba.getStability() - cuba.getInfluence(Side::USSR);
  // cuba.addInfluence(Side::USSR, std::max(0, sufficientInfluence));
  // return true;
  std::vector<CommandPtr> commands;
  // TODO: Fidel card event implementation
  // This requires special commands to handle clearing influence and adding
  // control For now, return empty as requested in task
  return commands;
}

bool Fidel::canEvent(const Board& board) const {
  return true;
}

std::vector<CommandPtr> NuclearTestBan::event(Side side,
                                              const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<LambdaCommand>([side](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (current_defcon - 2);
    board.pushState(std::make_shared<ChangeVpCommand>(side, vp_change));
  }));
  commands.push_back(std::make_shared<ChangeDefconCommand>(2));
  return commands;
}

bool NuclearTestBan::canEvent(const Board& board) const {
  return true;
}

// 特殊な影響力配置カードの実装
std::vector<CommandPtr> Comecon::event(Side side,
                                       const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 1;
        config.allowedRegions = std::vector<Region>{Region::EAST_EUROPE};
        config.excludeOpponentControlled = true;  // 非US支配国のみ
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool Comecon::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> Decolonization::event(Side side,
                                              const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 1;
        config.allowedRegions =
            std::vector<Region>{Region::AFRICA, Region::SOUTH_EAST_ASIA};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool Decolonization::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> DestaLinization::event(Side side,
                                               const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  // De-Stalinizationは再配置だが、今回は配置のみ実装
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 2;
        config.allowedRegions = std::nullopt;     // 全地域OK
        config.excludeOpponentControlled = true;  // 非US支配国のみ
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool DestaLinization::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> ColonialRearGuards::event(
    Side side, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 1;
        config.allowedRegions =
            std::vector<Region>{Region::AFRICA, Region::SOUTH_EAST_ASIA};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool ColonialRearGuards::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> PuppetGovernments::event(Side side,
                                                 const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 3;
        config.maxPerCountry = 1;
        config.allowedRegions = std::nullopt;  // 全地域OK
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = true;  // 影響力のない国のみ

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool PuppetGovernments::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> OASFounded::event(Side side,
                                          const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 2;
        config.maxPerCountry = 0;  // 無制限
        config.allowedRegions =
            std::vector<Region>{Region::CENTRAL_AMERICA, Region::SOUTH_AMERICA};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool OASFounded::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> LiberationTheology::event(
    Side side, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 3;
        config.maxPerCountry = 2;
        config.allowedRegions = std::vector<Region>{Region::CENTRAL_AMERICA};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool LiberationTheology::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> WarsawPactFormed::event(Side side,
                                                const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  // Warsaw Pact Formedは2つの選択肢があるが、今回は配置の部分のみ実装
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 5;
        config.maxPerCountry = 2;
        config.allowedRegions = std::vector<Region>{Region::EAST_EUROPE};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool WarsawPactFormed::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> MarshallPlan::event(Side side,
                                            const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 7;
        config.maxPerCountry = 1;
        config.allowedRegions = std::vector<Region>{Region::WEST_EUROPE};
        config.excludeOpponentControlled = true;  // 非USSR支配国のみ
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool MarshallPlan::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> UssuriRiverSkirmish::event(
    Side side, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  // Ussuri River Skirmishは条件分岐があるが、今回は配置の部分のみ実装
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 2;
        config.allowedRegions = std::vector<Region>{Region::ASIA};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool UssuriRiverSkirmish::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> TheReformer::event(Side side,
                                           const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  // The Reformerは条件によって配置数が変わるが、今回は4個の場合のみ実装
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;  // 条件により6個の場合もある
        config.maxPerCountry = 2;
        config.allowedRegions = std::vector<Region>{Region::EUROPE};
        config.excludeOpponentControlled = false;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}

bool TheReformer::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> SpecialRelationship::event(Side side,
                                                   const Board& board) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        std::vector<std::shared_ptr<Move>> moves;

        // UK支配をチェック
        bool uk_controlled = board.getWorldMap()
                                 .getCountry(CountryEnum::UNITED_KINGDOM)
                                 .getControlSide() == Side::USA;

        if (!uk_controlled) {
          return moves;  // UK支配でない場合は空のmovesを返す
        }

        // NATO有効かチェック
        bool nato_active =
            board.getCardEffectsInProgress().contains(CardEnum::NATO);

        if (nato_active) {
          // NATO有効時: 西欧地域の各国に+2
          const auto& western_europe_countries =
              board.getWorldMap().countriesInRegion(Region::WEST_EUROPE);
          moves.reserve(western_europe_countries.size());

          for (const auto& country : western_europe_countries) {
            moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
                card_enum, side,
                std::map<CountryEnum, int>{{country.getId(), 2}}));
          }
        } else {
          // NATO無効時: UK隣接国に+1
          const auto& united_kingdom =
              board.getWorldMap().getCountry(CountryEnum::UNITED_KINGDOM);
          const auto uk_adjacent = united_kingdom.getAdjacentCountries();
          moves.reserve(uk_adjacent.size());

          for (const auto& country_enum : uk_adjacent) {
            moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
                card_enum, side,
                std::map<CountryEnum, int>{{country_enum, 1}}));
          }
        }

        return moves;
      }));

  // VP+2はNATO有効時のみプッシュ
  bool nato_active = board.getCardEffectsInProgress().contains(CardEnum::NATO);
  if (nato_active) {
    commands.emplace_back(std::make_shared<ChangeVpCommand>(Side::USA, 2));
  }

  return commands;
}

bool SpecialRelationship::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> SouthAfricanUnrest::event(
    Side side, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USSR,
      [card_enum = getId(), side = Side::USSR](
          const Board& /*board*/) -> std::vector<std::shared_ptr<Move>> {
        std::vector<std::shared_ptr<Move>> moves;
        moves.reserve(4);

        // 1. South Africa に +2
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::SOUTH_AFRICA, 2}}));

        // 2. South Africa +1, Angola +1, Botswana +1
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::SOUTH_AFRICA, 1},
                                       {CountryEnum::ANGOLA, 1},
                                       {CountryEnum::BOTSWANA, 1}}));

        // 3. South Africa +1, Angola +2
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::SOUTH_AFRICA, 1},
                                       {CountryEnum::ANGOLA, 2}}));

        // 4. South Africa +1, Botswana +2
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::SOUTH_AFRICA, 1},
                                       {CountryEnum::BOTSWANA, 2}}));

        return moves;
      }));
  return commands;
}

bool SouthAfricanUnrest::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> Junta::event(Side side, const Board& /*board*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      side,
      [card_enum = getId(),
       side](const Board& board) -> std::vector<std::shared_ptr<Move>> {
        std::vector<std::shared_ptr<Move>> moves;

        // 中南米地域の全国を列挙
        const auto& central_america_countries =
            board.getWorldMap().countriesInRegion(Region::CENTRAL_AMERICA);
        const auto& south_america_countries =
            board.getWorldMap().countriesInRegion(Region::SOUTH_AMERICA);

        moves.reserve(central_america_countries.size() +
                      south_america_countries.size());

        // 中米の各国に2影響力を配置するEventPlaceInfluenceMoveを生成
        for (const auto& country : central_america_countries) {
          moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
              card_enum, side,
              std::map<CountryEnum, int>{{country.getId(), 2}}));
        }

        // 南米の各国に2影響力を配置するEventPlaceInfluenceMoveを生成
        for (const auto& country : south_america_countries) {
          moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
              card_enum, side,
              std::map<CountryEnum, int>{{country.getId(), 2}}));
        }

        return moves;
      }));
  return commands;
}

bool Junta::canEvent(const Board& /*board*/) const {
  return true;
}
