![CI](https://github.com/Twilight-Struggle/TSGE/workflows/CI/badge.svg)
[![codecov](https://codecov.io/gh/Twilight-Struggle/TSGE/branch/main/graph/badge.svg)](https://codecov.io/gh/Twilight-Struggle/TSGE)

# TSGE (Twilight Struggle Game Engine)

TSGEはボードゲーム「Twilight Struggle」のゲームエンジンであり、C++20で実装された学習・研究用途向け基盤です。

- 深層学習および強化学習アルゴリズムの訓練
- モンテカルロ木探索（MCTS）ベースのAI実装
- テストや評価に耐える高速なゲームプレイシミュレーション

## 特長

[各コンポーネントの関係](docs/flowchart.md)

- **Command/Move/Boardの三層分離**: プレイヤー入力（Move）と状態変更（Command）をBoardから切り離し、状態遷移の検証とMCTSコピー最適化を両立。
- **PhaseMachineによるフェーズ駆動のゲームフロー**: PhaseMachineがMoveをCommand列へ変換し、ターン進行とフェーズ遷移を明示的に制御。
- **MCTS向けのBoard最適化**: Board::copyForMCTS()により情報隠蔽を保ちつつ高速コピーし、RandomizerやDeck状態も安全に複製。
- **ポリシーベース設計のGame/Player**: Gameがポリシー差し替えを許容し、TsNnMctsPolicyなど学習エージェントを容易に実装・比較可能。

## 主要コンポーネントと連携

[連携の流れ](docs/sequenceDiagram.md)

1. PlayerがLegalMovesGeneratorから提供される合法手を受け取り、Moveを選択。
2. MoveはPhaseMachineに渡され、Card IDやSide情報を保持したまま、Command列へと展開される。
3. PhaseMachineはGameLogicLegalMovesGenerator/CardEffectLegalMoveGeneratorと連携してMoveの妥当性を確認し、CommandをBoardに適用。
4. BoardはWorldMap、Trackers、Deck、Cards、Randomizerなどのゲーム状態を一元管理し、Commandのapply()によってのみ更新される。
5. 更新されたBoardはGameを通じてPlayerに再提示され、学習ループやMCTSシミュレーションに供される。

## サポートモジュールとMCTS最適化

[MCTS時のBoardの変化](docs/MCTSflowchart.md)

- **WorldMap/Country/Trackers**: 地政学的エリアと宇宙開発・DEFCONなどのトラックを分離管理し、Boardはポインタ/参照で保持して高速コピーを実現。
- **Deck/Cards**: 共通ヘッダで全カードにアクセスし、カードカテゴリごとの実装を分割することでイベント追加や除去が容易。
- **Randomizer**: Boardメンバとして保持し、MCTSコピー時にも同一系列を再現できるよう設計。
- **Playerポリシー**: `players/`配下のポリシーを差し替えるだけでZero系MCTSやテスト用ダミープレイヤーを注入可能。

## ビルドとテスト

```bash
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build
```

詳細な開発フローやカード実装パターンは`CLAUDE.md`および各ディレクトリの設計ドキュメントを参照してください。
