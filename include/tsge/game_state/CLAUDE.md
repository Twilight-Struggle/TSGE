# game_state の設計要約

## 中核データ
- `Country`: `CountryEnum` をキーに安定度・隣接国・所属地域を参照しつつ、USSR/USA の影響力を保持・調整する。`getControlSide` が支配判定、`getRegions`/`getAdjacentCountries` が隣接探索の入口。
- `WorldMap`: 86 件の `Country` を生成して保持し、地域ごとの集合も同時管理する。初期化時に `INITIAL_INFLUENCE_DATA` を反映し、`placeableCountries` で Ops 配置先の候補を列挙する。
- `Deck`: `Randomizer` とカードプールを参照し、山札・捨て札・除外済みカードの 3 つの `std::vector<CardEnum>` を運用する。時期ごとのカード追加とリシャッフルが主な責務。
- `world_map_constants`: 各 `Country` の静的定義と初期影響力テーブルをまとめ、`WorldMap`/`Country` の前提値を提供する。

## カード層
- `Card`: カード ID・Ops 値・陣営・時期・除外フラグを保持し、`event` で `CommandPtr` 列を生成する抽象基底。`canEvent` がイベント可否チェック。
- `cards.hpp/cpp`: 代表的カードの具象 `event` 実装を提供。

## トラック群
- `SpaceTrack`: 宇宙進捗と挑戦回数を管理し、段階ごとの Ops 条件・VP 返還および追加行動権判定を提供。
- `DefconTrack`: DEFCON 値を 1〜5 でクランプしながら増減。
- `MilopsTrack`: 陣営別軍事 Ops を最大 5 まで蓄積し、ターン開始時にリセット。
- `TurnTrack`: 現在ターンと配布枚数（全 10 ターン）を管理。
- `ActionRoundTrack`: ターンごとの行動回数上限と宇宙進捗由来の追加行動権を同期し、リセット API も提供。

## Move との接続
- 派生 `Move`（影響力配置・クーデター・リアラインメント・宇宙競争・イベント等）は、保持する `CardEnum` と `Side` を `Card` の `event`/Ops 消費に橋渡しし、`toCommand` で上記ゲーム状態を更新する `Command` 列を生成する。これにより `Country`/`WorldMap`/各トラックの状態が一貫して更新される。
