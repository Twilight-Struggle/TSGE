# 実装計画コマンド

## 目的
コードを書く前に実装計画を立てる。

## `.claude/work/scratchpads/scratchpad_$ARGUMENTS.md` の構成
```md
# (題名を考えて記載)

## 読み込み検証すべきファイル
- 
- 
...

## 実装計画

## 実装手順
```

## 手順
1. タスクファイルを読む: `.claude/work/tasks/task_$ARGUMENTS.md`
2. 既存のスクラッチパッドがあれば読む: `.claude/work/scratchpads/scratchpad_$ARGUMENTS.md`
3. `task_$ARGUMENTS.md`、`scratchpad_$ARGUMENTS.md`を実現するうえで、コードベース上必要なファイルを読み、`scratchpad_$ARGUMENTS.md`の`読み込み検証すべきファイル`を修正(正規表現等の省略記法を使って構わない)
4. `scratchpad_$ARGUMENTS.md` ファイルに`読み込み検証すべきファイル`を参照しつつ、詳細な実装計画を`実装計画`にを修正
5. `scratchpad_$ARGUMENTS.md` ファイルに`実装手順`に後でClaude codeが実装しやすいように実装計画をTODO形式で記載
6. コード実装に進む前に人間によるレビューを待つ
7. 人間がフィードバックを `task_$ARGUMENTS.md` に追加した場合（#追記1, #追記2 などでマーク）、それに従って `scratchpad_$ARGUMENTS.md` を修正する(もはや不必要な部分は削除しても良いし、純粋な追記でも良い)

## 出力
すべての計画内容は `.claude/work/scratchpads/scratchpad_$ARGUMENTS.md` に書き込む必要がある。
また `scratchpad_$ARGUMENTS.md` への書き込みコードはUTF-8とし、文字化けが発生しないよう注意すること。
計画がレビューされ承認されるまでコードを書かないこと。