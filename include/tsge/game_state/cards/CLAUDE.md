# Cards ディレクトリ

このディレクトリには、Twilight Struggleのカード実装が整理されています。

## ディレクトリ構造

```
cards/
├── README.md                   # このファイル
├── scoring_cards.hpp/cpp       # スコアリングカード
├── basic_event_cards.hpp/cpp   # 基本的なイベントカード
└── special_cards.hpp/cpp       # 特殊なカード
```

## カード分類基準

### スコアリングカード (`scoring_cards.hpp/cpp`)
VP計算を伴う地域スコアリングカード。

**例:**
- AsiaScoring, EuropeScoring, MiddleEastScoring
- SoutheastAsiaScoring（特殊な計算ロジック）

**特徴:**
- `ScoreRegionCommand`または`SoutheastAsiaScoringCommand`を使用
- 単一の効果のみ
- canEventは常にtrue

### 基本イベントカード (`basic_event_cards.hpp/cpp`)
単純な効果を持つカード。

**例:**
- DuckAndCover, NuclearTestBan（VP/DEFCON変更）
- Comecon, Decolonization（特定条件での影響力配置）
- WarsawPactFormed, MarshallPlan（地域限定の影響力配置）

**特徴:**
- 単一または少数のCommandを生成
- 標準的な`RequestCommand`や`LambdaCommand`を使用
- `CardSpecialPlaceInfluenceConfig`を使った影響力配置

### 特殊カード (`special_cards.hpp/cpp`)
プレイヤーに複数の選択肢を提示するカード。

**例:**
- SpecialRelationship（4カ国から1つ選択）
- SouthAfricanUnrest（4つの配置パターンから選択）

**特徴:**
- `RequestCommand`で複数の`EventPlaceInfluenceMove`を生成
- 各選択肢は異なる効果を持つ
- より複雑なゲームロジック

## 新しいカードの追加ガイドライン

### 1. カードの分類
まず、カードがどのカテゴリに属するか判断します：

- **単純な効果のみ** → `basic_event_cards.hpp/cpp`
- **地域スコアリング** → `scoring_cards.hpp/cpp`
- **複雑な効果** → `special_cards.hpp/cpp`

### 2. ヘッダファイルへの追加
該当するヘッダファイル（例：`basic_event_cards.hpp`）にクラス宣言を追加：

```cpp
class NewCard final : public Card {
 public:
  NewCard()
      : Card(CardEnum::NEW_CARD, "New Card", 3, Side::USA,
             WarPeriod::MID_WAR, false) {}
  [[nodiscard]]
  std::vector<CommandPtr> event(Side side, const Board& board) const override;
  [[nodiscard]]
  bool canEvent(const Board& board) const override;
};
```

### 3. 実装ファイルへの追加
対応する`.cpp`ファイル（例：`basic_event_cards.cpp`）に実装を追加：

```cpp
std::vector<CommandPtr> NewCard::event(Side side, const Board& board) const {
  std::vector<CommandPtr> commands;
  // カード効果の実装
  return commands;
}

bool NewCard::canEvent(const Board& board) const {
  // イベント発動条件の実装
  return true;
}
```

### 4. CardPoolへの登録
gameインスタンスで作成されるCardPoolに登録(将来的)

### 5. テストの追加
`tests/game_state/cards_test.cpp`または該当するテストファイルにテストを追加します。

## 実装パターン

### 影響力配置パターン
`CardSpecialPlaceInfluenceConfig`を使った影響力配置：

```cpp
std::vector<CommandPtr> MyCard::event(Side side, const Board& board) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        CardSpecialPlaceInfluenceConfig config;
        config.totalInfluence = 4;
        config.maxPerCountry = 1;
        config.allowedRegions = std::vector<Region>{Region::EUROPE};
        config.excludeOpponentControlled = true;
        config.onlyEmptyCountries = false;

        return LegalMovesGenerator::generateCardSpecificPlaceInfluenceMoves(
            board, side, card_enum, config);
      }));
  return commands;
}
```

### VP/DEFCON変更パターン
単純なVPやDEFCON変更：

```cpp
std::vector<CommandPtr> MyCard::event(Side side, const Board& board) const {
  std::vector<CommandPtr> commands;
  commands.push_back(std::make_shared<ChangeVpCommand>(Side::USA, 2));
  commands.push_back(std::make_shared<ChangeDefconCommand>(1));
  return commands;
}
```

### 選択肢パターン
プレイヤーに複数の選択肢を提示：

```cpp
std::vector<CommandPtr> MyCard::event(Side side, const Board& board) const {
  std::vector<CommandPtr> commands;
  commands.emplace_back(std::make_shared<RequestCommand>(
      Side::USA,
      [card_enum = getId(), side = Side::USA](
          const Board& board) -> std::vector<std::shared_ptr<Move>> {
        std::vector<std::shared_ptr<Move>> moves;
        moves.reserve(2);

        // 選択肢1
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::FRANCE, 2}}));

        // 選択肢2
        moves.emplace_back(std::make_shared<EventPlaceInfluenceMove>(
            card_enum, side,
            std::map<CountryEnum, int>{{CountryEnum::UK, 2}}));

        return moves;
      }));
  return commands;
}
```

## 注意事項

### インクルード
各カテゴリのヘッダファイルは`cards.hpp`によって自動的にインクルードされます。
既存のコードは`#include "tsge/game_state/cards.hpp"`だけで全カードにアクセスできます。

### コンパイル
新しいカードを追加した後は必ずビルドとテストを実行してください：

```bash
cmake --build build
ctest --test-dir build
```

### カバレッジ
`CMakeLists.txt`のカバレッジターゲット設定も必要に応じて更新してください。
`ci.yml`についても同様。