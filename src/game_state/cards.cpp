// どこで: src/game_state/cards.cpp
// 何を: CARD.mdの仕様に沿ってカードイベントを実装し、Boardコマンド列を構築する
// なぜ: イベント効果を一元化し、テスト容易性と再利用性を確保するため
#include "tsge/game_state/cards.hpp"

#include <memory>

#include "tsge/actions/command.hpp"
#include "tsge/actions/legal_moves_generator.hpp"
#include "tsge/core/board.hpp"
#include "tsge/enums/game_enums.hpp"

std::vector<CommandPtr> RegionScoringCard::event(Side /*side*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<ScoreRegionCommand>(region_));
  return commands;
}

bool RegionScoringCard::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> SoutheastAsiaScoring::event(Side /*side*/) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<SoutheastAsiaScoringCommand>());
  return commands;
}

bool SoutheastAsiaScoring::canEvent(const Board& /*board*/) const {
  return true;
}

std::vector<CommandPtr> DuckAndCover::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<ChangeDefconCommand>(-1));
  commands.push_back(std::make_shared<LambdaCommand>([](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (5 - current_defcon);
    board.pushState(std::make_shared<ChangeVpCommand>(Side::USA, vp_change));
  }));
  return commands;
}

std::vector<CommandPtr> ChinaCard::event(Side /*side*/) const {
  return {};
}

bool ChinaCard::canEvent(const Board& /*board*/) const {
  return false;
}

bool DuckAndCover::canEvent(const Board& board) const {
  return true;
}

std::vector<CommandPtr> Fidel::event(Side side) const {
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

bool NuclearTestBan::canEvent(const Board& board) const {
  return true;
}

std::vector<CommandPtr> NuclearTestBan::event(Side side) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<LambdaCommand>([side](Board& board) {
    int current_defcon = board.getDefconTrack().getDefcon();
    int vp_change = (current_defcon - 2);
    board.pushState(std::make_shared<ChangeVpCommand>(side, vp_change));
  }));
  commands.push_back(std::make_shared<ChangeDefconCommand>(2));
  return commands;
}

// 特殊な影響力配置カードの実装
std::vector<CommandPtr> Comecon::event(Side side) const {
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

std::vector<CommandPtr> Decolonization::event(Side side) const {
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

std::vector<CommandPtr> DestaLinization::event(Side side) const {
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

std::vector<CommandPtr> ColonialRearGuards::event(Side side) const {
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

std::vector<CommandPtr> PuppetGovernments::event(Side side) const {
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

std::vector<CommandPtr> OASFounded::event(Side side) const {
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

std::vector<CommandPtr> LiberationTheology::event(Side side) const {
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

std::vector<CommandPtr> WarsawPactFormed::event(Side side) const {
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

std::vector<CommandPtr> MarshallPlan::event(Side side) const {
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

std::vector<CommandPtr> UssuriRiverSkirmish::event(Side side) const {
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

std::vector<CommandPtr> TheReformer::event(Side side) const {
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
