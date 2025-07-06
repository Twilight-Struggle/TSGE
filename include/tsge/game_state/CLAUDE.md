# gane_stateの設計
## country
トワイライトストラグルに登場する国を表すクラス。
WorldMapを通じてアクセスされることが前提。

### テスト
適当な国を作成し、bool operator<(const Country& other) const { return id_ < other.id_; }
以外のメソッドに対するテストを作成。
[[unlikely]]のカバレッジは考えなくて良い。

## world_map
Countryを管理するクラス。

### テスト
適当なCountryに対して各メソッドをテスト。

## card
登場するカードの基底クラス。

### テスト
仮想クラスなのでテスト不可。

## cards
詳細は`cards.md`に記載予定

## trackers
ボードのトラッカーを表す。

### テスト
各クラスの実装にそってカバレッジを満たすようにテストを実装