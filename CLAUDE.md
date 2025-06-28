# CLAUDE.md
Claudeは必ず日本語で回答するように。プロジェクトの自然言語は日本語とする。

このファイルは、このリポジトリのコードを扱う際のClaude Code (claude.ai/code) 向けのガイダンスを提供します。

## Project Overview

TSGE（Twilight Struggle Game Engine）は、ボードゲーム「Twilight Struggle」用のC++20ゲームエンジンで、以下の用途に設計されています：

- 深層学習と強化学習
- モンテカルロ木探索（MCTS）AI実装
- 訓練のための高速ゲームプレイシミュレーション

## Build Commands

```bash
# プロジェクトの設定（初回もしくはbuildフォルダ初期化後、CMakeLists.txt更新後など）
cmake -B build -G Ninja

# プロジェクトのビルド
cmake --build build

# 本番版のビルド（テストマクロなし）
cmake -B build -G Ninja -DPROD=ON
cmake --build build

# 全テストの実行
ctest --test-dir build

# coverage test
cmake -B build -G Ninja -DCOVERAGE=ON
cmake --build build
cmake --build build --target coverage

# clang-tidyの実行
## 単一ファイルをチェック(ファイル名は例)
clang-tidy -p build src/core/game.cpp

# 特定のテストの実行
./build/command_test
./build/world_map_test
./build/trackers_test
./build/cards_test
```

## アーキテクチャ概要

エンジンは、ゲーム状態、アクション、ゲームフローの明確な分離に従います：

### Core Components

1. **Board** (`board.hpp`): 中央ゲーム状態コンテナ
   - 全ゲーム状態を保持： cards, world map, tracks, player hands, victory points
   - MCTSコピー性能のために最適化
   - Commandオブジェクトを通してのみ変更可能

2. **Command** (`command.hpp`): 状態変更パターン
   - `apply()`メソッドを通してのみBoard状態を変更できる唯一のクラス
   - 派生クラス： `ActionPlaceInfluence`など
   - `Request` クラスはプレイヤー入力要求を処理

3. **Move** (`move.hpp`): 軽量アクション表現
   - プレイヤー-ゲーム間の通信に使用
   - `toCommand()`メソッドを通してCommand(s)に変換
   - 1つのMoveから複数のCommandを生成可能

4. **Game** (`game.hpp`): トップレベルゲーム統制者
   - BoardとPlayerの相互作用を管理
   - テストの柔軟性のためポリシーベース設計を使用
   - ゲームフローをPhaseMachineに委譲

5. **PhaseMachine** (`phase_machine.hpp`): ゲームフロー制御
   - MoveをCommandに変換して適用
   - ゲームフェーズと遷移を管理
   - LegalMovesGeneratorから合法手を要求

6. **LegalMovesGenerator** (`legal_moves_generator.hpp`): 合法手の検証
   - 現在のゲーム状態での全合法手を生成
   - 現在のフェーズに基づくコンテキスト認識

### サポートコンポーネント

- **Country** (`country.hpp`): 個別国家状態
- **WorldMap** (`world_map.hpp`): 全国家のコンテナ
- **Trackers** (`trackers.hpp`): 各種ゲームトラック（宇宙、DEFCON等）
- **Player** (`player.hpp`): 合法手を受け取り、選択したMoveを返す
- **Cards** (`cards.hpp`): カード定義と管理

## フォルダ構成

プロジェクトは以下の階層構造に整理されています：

```
include/tsge/
├── core/          # コアゲームメカニクス（board, game, phase_machine）
├── actions/       # アクションシステム（command, move, legal_moves_generator）
├── game_state/    # ゲーム状態管理（country, world_map, cards, trackers）
├── players/       # プレイヤー関連（player, policies）
├── utils/         # ユーティリティ（randomizer）
└── enums/         # 共通列挙型（game_enums）

src/                # 実装ファイル（include/と同じ階層構造、tsge/は含まない）
tests/              # テストファイル（機能別にサブディレクトリ分け）
```

### インクルードパス
- ヘッダファイルは`#include "tsge/モジュール名/ファイル名.hpp"`の形式
- 例：`#include "tsge/core/board.hpp"`、`#include "tsge/actions/command.hpp"`

## 開発ノート

### Testing
- プロジェクトはGoogle Testフレームワークを使用
- テストビルドはデフォルトで有効（無効にするには`-DPROD=ON`を使用）
- テスト固有コードに対する`#ifdef TEST`を使った条件コンパイル

### コード変更
- 各コンポーネントの責任範囲を明確に定義し、疎結合を維持することが重要
- Board状態の変更はCommandオブジェクトを通す必要がある
- 新しいCommand/Moveタイプについては既存パターンに従う
- BoardコンポーネントModifying時はMCTSコピー効率を維持
- MCTSのコピー効率を最適化するためには、Boardのデータ構造を慎重に設計する必要がある

### clang-tidy関連
- **警告の抑制**：設計上安全が保証されている配列アクセス等には`// NOLINTNEXTLINE(warning-name)`を使用

### claudeが分かりづらいところ
- **policies.hpp**はplayersディレクトリに配置：DecisionPolicy（TestPolicy等）はPlayerから利用される構造のため
- **unique_ptr使用時の注意**：`std::vector<std::unique_ptr<T>>`で`insert()`や範囲ベースコピーは禁止。`std::move()`と`emplace_back()`を使用する

### パフォーマンス最適化ガイドライン
- **constexpr活用**：Side enumを使った関数（getOpponentSide, getVpMultiplier等）はconstexpr化により配列アクセスにしてコンパイル時計算を可能にする
- **ブランチ予測最適化**：関数内部の分岐では[[unlikely]]属性を稀なケース（エラー処理、早期リターン等）に使用する
- **vectorの事前確保**：LegalMovesGenerator等でemplace_backループ前にreserve()を行い、メモリ再確保を削減する
- **小さなメソッドのインライン化**：Country::addInfluence()等の3-5行程度で頻繁に呼ばれるメソッドはヘッダに移動してインライン化する
- **不要なチェックは削除**:実装の前提条件（例：realignment historyは必ず1つ以上の要素を持つ）を確認し、不要なチェックは削除
- **配列アクセスの最適化**：Side列挙型（USSR=0, USA=1）による`std::array<T,2>`への配列アクセスは境界外アクセスがありえないため、`.at()`ではなく`[]`を使用。MCTSシミュレーションでのパフォーマンスを優先

## 将来計画(claudeは読まなくて良い)
- gitflowとCI
- テスト整理
- WorldMapCountry定数括りだし
- メモリアクセスパターンの最適化→キャッシュの関係から同じタイミングでアクセスされやすいデータは近くの方がいい。以下例。
```cpp
class SpaceTrack {
  std::array<int, 2> spaceTrack_ = {0, 0};
  std::array<int, 2> spaceTried_ = {0, 0};
};
```
↓
```cpp
class SpaceTrack {
  struct SideData {
    int track = 0;
    int tried = 0;
  };
  std::array<SideData, 2> data_;
};
```